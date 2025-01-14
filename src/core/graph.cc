#include "core/graph.h"
#include "core/common.h"
#include "core/object.h"
#include "core/ref.h"
#include "core/runtime.h"
#include "operators/matmul.h"
#include "operators/transpose.h"
#include "utils/print.hpp"
#include <algorithm>
#include <cstddef>
#include <map>
#include <numeric>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace infini {

void GraphObj::addOperatorAndConnect(const Operator &op) {
  sorted = false;
  ops.push_back(op);
  for (auto &input : op->getInputs()) {
    if (input) {
      input->addTarget(op);
      if (auto pred = input->getSource()) {
        pred->addSuccessors(op);
        op->addPredecessors(pred);
      }
    }
  }
  for (auto &output : op->getOutputs()) {
    if (output) {
      output->setSource(op);
      for (auto &succ : output->getTargets()) {
        succ->addPredecessors(op);
        op->addSuccessors(succ);
      }
    }
  }
}

string GraphObj::toString() const {
  std::ostringstream oss;
  oss << "┌─[Graph Tensors]" << "\n";
  for (const auto &tensor : tensors)
    oss << "│" << (&tensor == &tensors.back() ? "   └" : "   ├") << "───"
        << tensor << "\n";

  oss << "└─[Graph operators]" << "\n";
  for (const auto &op : ops) {
    vector<UidBaseType> preds, succs;
    for (auto &o : op->getPredecessors())
      preds.emplace_back(o->getGuid());
    for (auto &o : op->getSuccessors())
      succs.emplace_back(o->getGuid());
    oss << (&op == &ops.back() ? "    └" : "    ├") << "───";
    oss << "{OP: " << op->getGuid();
    oss << ", pred: " << vecToString(preds);
    oss << ", succ: " << vecToString(succs);
    oss << ", " << op << "}\n";
  }

  return oss.str();
}

bool GraphObj::topo_sort() {
  if (this->sorted) {
    return true;
  }
  std::vector<Operator> sorted;
  std::unordered_set<OperatorObj *> flags;
  sorted.reserve(ops.size());
  flags.reserve(ops.size());
  while (sorted.size() < ops.size()) {
    // Any node is move to sorted in this loop.
    auto modified = false;
    for (auto const &op : ops) {
      if (auto const &inputs = op->getInputs();
          flags.find(op.get()) == flags.end() &&
          std::all_of(inputs.begin(), inputs.end(),
                      [&flags](auto const &input) {
                        auto ptr = input->getSource().get();
                        return !ptr || flags.find(ptr) != flags.end();
                      })) {
        modified = true;
        sorted.emplace_back(op);
        flags.insert(op.get());
      }
    }
    if (!modified) {
      return false;
    }
  }
  this->ops = std::move(sorted);
  return this->sorted = true;
}

void GraphObj::optimize() {
  using GuidType = decltype(ops[0]->getGuid());

  std::unordered_map<GuidType, Operator> op_map;
  std::unordered_map<GuidType, Tensor> tensor_map;
  std::unordered_map<GuidType, size_t> op_i_map, tensor_i_map;
  std::unordered_set<GuidType> erased_ops, erased_tensors;

  size_t op_i = 0;
  for (auto &op : ops) {
    op_map[op->getGuid()] = op;
    op_i_map[op->getGuid()] = op_i++;
  }

  size_t tensor_i = 0;
  for (auto &tensor : tensors) {
    tensor_map[tensor->getGuid()] = tensor;
    tensor_i_map[tensor->getGuid()] = tensor_i++;
  }

  // 1. 计划合并 transpose
  for (const auto &[guid, op] : op_map) {
    if (erased_ops.find(guid) != erased_ops.end())
      continue;

    if (op->getOpType() == OpType::Transpose) {
      auto in = op->getInputs(0);
      auto out = op->getOutput(0);
      auto next_op = out->getTargets()[0];

      if (next_op->getOpType() == OpType::Transpose) {
        // 先解析为 Ref<TransposeObj>
        auto tr_op = as<TransposeObj>(op);
        auto next_tr_op = as<TransposeObj>(next_op);

        // 只有两次的 permute 相同, 才能合并
        if (tr_op->getPermute() == next_tr_op->getPermute()) {
          auto next_out = next_op->getOutput(0);
          auto next_next_op = next_out->getTargets()[0];

          // 根据测试样例, 要把 next_next_op 的输入替换为 curr_op 的输入
          op_map[next_next_op->getGuid()]->replaceInput(next_out, in);

          // targets 也要替换
          auto in_i = tensor_i_map[in->getGuid()];
          tensors[in_i]->removeTarget(op);
          tensors[in_i]->addTarget(next_next_op);

          // 更新 pred
          op_map[next_next_op->getGuid()]->removePredecessors(next_op);

          // 更新计划
          erased_ops.insert(guid);
          erased_ops.insert(next_op->getGuid());
          op_i_map.erase(guid);
          op_i_map.erase(next_op->getGuid());
          tensor_i_map.erase(out->getGuid());
          tensor_i_map.erase(next_out->getGuid());
        }
      }
    }
  }

  // 2. 计划将 transpose 融入到 matmul 中
  for (const auto &[guid, op] : op_map) {
    if (erased_ops.find(guid) != erased_ops.end())
      continue;

    if (op->getOpType() == OpType::Transpose) {
      auto in = op->getInputs(0);
      auto out = op->getOutput(0);
      auto next_op = out->getTargets()[0];

      // 解析为 Ref<TransposeObj>
      auto curr_tr_op = as<TransposeObj>(op);

      auto curr_perm = curr_tr_op->getPermute();
      auto is_last_two_dims_swapped =
          curr_perm.size() >= 2 &&
          curr_perm[curr_perm.size() - 1] == (int)(curr_perm.size() - 2) &&
          curr_perm[curr_perm.size() - 2] == (int)(curr_perm.size() - 1);

      if (next_op->getOpType() == OpType::MatMul && is_last_two_dims_swapped) {
        // 先解析为 Ref<MatmulObj>
        auto mul_op = as<MatmulObj>(next_op);

        // 区分 curr_out 是 A 还是 B, 分情况讨论
        if (out->getGuid() == mul_op->getInputs(0)->getGuid()) {
          mul_op->setTransA(!mul_op->getTransA());
        } else {
          mul_op->setTransB(!mul_op->getTransB());
        }

        // 根据测试样例, 还要把对应的 A / B 直接替换为 curr_in
        op_map[mul_op->getGuid()]->replaceInput(op->getOutput(0), in);

        // targets 也要替换
        auto in_i = tensor_i_map[in->getGuid()];
        tensors[in_i]->removeTarget(op);
        tensors[in_i]->addTarget(next_op);

        // 更新 pred
        op_map[next_op->getGuid()]->removePredecessors(op);

        // 更新计划
        erased_ops.insert(guid);
        op_i_map.erase(guid);
        tensor_i_map.erase(out->getGuid());
      }
    }
  }

  // 3. 应用计划
  TensorVec new_tensors;
  OpVec new_ops;
  new_tensors.reserve(op_i_map.size());
  new_ops.reserve(op_i_map.size());
  for (const auto &[guid, tensor_i] : tensor_i_map) {
    new_tensors.emplace_back(std::move(tensors[tensor_i]));
  }
  for (const auto &[guid, op_i] : op_i_map) {
    new_ops.emplace_back(std::move(ops[op_i]));
  }
  tensors = std::move(new_tensors);
  ops = std::move(new_ops);
}

Tensor GraphObj::getTensor(int fuid) const {
  for (auto tensor : tensors) {
    if (tensor->getFuid() == fuid) {
      return tensor;
    }
  }
  return nullptr;
}

void GraphObj::shape_infer() {
  for (auto &op : ops) {
    auto ans = op->inferShape();
    IT_ASSERT(ans.has_value());
    auto oldOutputs = op->getOutputs();
    IT_ASSERT(ans.value().size() == oldOutputs.size());
    // replace the old output-shape and size with new one
    for (int i = 0; i < (int)ans.value().size(); ++i) {
      auto newShape = ans.value()[i];
      auto oldShape = oldOutputs[i]->getDims();
      auto fuid = oldOutputs[i]->getFuid();
      if (newShape != oldShape) {
        auto tensor = this->getTensor(fuid);
        tensor->setShape(newShape);
      }
    }
  }
}

void GraphObj::dataMalloc() {
  // topological sorting first
  IT_ASSERT(topo_sort() == true);

  vector<size_t> tensor_ptr_offsets;
  tensor_ptr_offsets.reserve(tensors.size());
  for (auto &tensor : tensors) {
    auto offset = allocator.alloc(tensor->getBytes());
    tensor_ptr_offsets.emplace_back(offset);
  }

  auto ptr = reinterpret_cast<char *>(allocator.getPtr());
  for (size_t i = 0; i < tensors.size(); ++i) {
    auto tensor_addr = reinterpret_cast<void *>(ptr + tensor_ptr_offsets[i]);
    tensors[i]->setDataBlob(make_ref<BlobObj>(runtime, tensor_addr));
  }

  allocator.info();
}

Tensor GraphObj::addTensor(Shape dim, DataType dtype) {
  return tensors.emplace_back(make_ref<TensorObj>(dim, dtype, runtime));
}

Tensor GraphObj::addTensor(const Tensor &tensor) {
  IT_ASSERT(tensor->getRuntime() == runtime,
            std::string("Tensor runtime mismatch: cannot add a tensor in ") +
                tensor->getRuntime()->toString() + " to " +
                runtime->toString());
  tensors.emplace_back(tensor);
  return tensor;
}

TensorVec GraphObj::addTensor(const TensorVec &tensors) {
  for (auto &t : tensors)
    addTensor(t);
  return tensors;
}

// tensor's "source" and "target" must be in "ops".
// tensor has no "source" and no "target" must not exist.
// "inputs" or "outputs" of operators must be in "tensors"
// "predecessors" and "successors" of an operator of "ops" must be in "ops".
bool GraphObj::checkValid() const {
  for (const auto &tensor : tensors) {
    IT_ASSERT(
        !(tensor->getTargets().size() == 0 && nullptr == tensor->getSource()));
    for (const auto &op : tensor->getTargets()) {
      IT_ASSERT(std::find(ops.begin(), ops.end(), op) != ops.end());
    }
    auto op = tensor->getSource();
    IT_ASSERT(!(op && std::find(ops.begin(), ops.end(), op) == ops.end()));
  }
  for (const auto &op : ops) {
    for (const auto &tensor : op->getInputs()) {
      IT_ASSERT(std::find(tensors.begin(), tensors.end(), tensor) !=
                tensors.end());
    }
    for (const auto &tensor : op->getOutputs()) {
      IT_ASSERT(std::find(tensors.begin(), tensors.end(), tensor) !=
                tensors.end());
    }
    for (const auto &pre : op->getPredecessors()) {
      IT_ASSERT(std::find(ops.begin(), ops.end(), pre) != ops.end());
    }
    for (const auto &suc : op->getSuccessors()) {
      IT_ASSERT(std::find(ops.begin(), ops.end(), suc) != ops.end());
    }
  }
  std::set<UidBaseType> s;
  // check whether two tensors with the same FUID exist
  for (const auto &tensor : tensors) {
    size_t cnt = s.count(tensor->getFuid());
    IT_ASSERT(cnt == 0, std::to_string(tensor->getFuid()));
    s.insert(tensor->getFuid());
  }
  return true;
}

} // namespace infini