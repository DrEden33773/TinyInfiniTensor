#include "core/graph.h"
#include "core/object.h"
#include "operators/matmul.h"
#include "operators/transpose.h"
#include "utils/print.hpp"
#include <algorithm>
#include <cstddef>
#include <map>
#include <numeric>
#include <queue>
#include <string>
#include <string_view>
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
  // 转成 map, 方便一些
  std::map<decltype(ops[0]->getGuid()), Operator> op_map;
  for (auto &op : ops) {
    op_map[op->getGuid()] = op;
  }

  // 1. 试图合并 transpose
  auto it = op_map.begin();
  while (it != op_map.end()) {
    auto guid = it->first;
    auto op = it->second;
    it++; // 提前移动, 防止 erase 之后迭代器失效

    if (op->getOpType() == OpType::Transpose) {
      auto in = op->getInputs(0);
      auto out = op->getOutput(0);
      if (out->getTargets().empty())
        continue;
      auto next_op = out->getTargets()[0];

      if (next_op->getOpType() == OpType::Transpose) {
        // 先转换为 TransposeObj *
        auto tr_op = dynamic_cast<TransposeObj *>(op.get());
        auto next_tr_op = dynamic_cast<TransposeObj *>(next_op.get());
        if (!tr_op || !next_tr_op)
          continue;

        // 只有两次的 permute 相同, 才能合并
        if (tr_op->getPermute() == next_tr_op->getPermute()) {
          auto next_out = next_op->getOutput(0);
          if (next_out->getTargets().empty())
            continue;
          auto next_next_op = next_out->getTargets()[0];

          // 根据测试样例, 要把 next_next_op 的输入替换为 curr_op 的输入
          op_map[next_next_op->getGuid()]->replaceInput(next_out, in);

          op_map.erase(guid);                // curr_op
          op_map.erase(next_op->getGuid());  // next_op
          op_map.erase(out->getGuid());      // curr_out
          op_map.erase(next_out->getGuid()); // next_out
        }
      }
    }
  }

  // 2. 试图将 transpose 融入到 matmul 中
  it = op_map.begin();
  while (it != op_map.end()) {
    auto guid = it->first;
    auto op = it->second;
    it++; // 提前移动, 防止 erase 之后迭代器失效

    if (op->getOpType() == OpType::Transpose) {
      auto in = op->getInputs(0);
      auto out = op->getOutput(0);
      if (out->getTargets().empty())
        continue;
      auto next_op = out->getTargets()[0];

      // 解析为 TransposeObj *
      auto curr_tr_op = dynamic_cast<TransposeObj *>(op.get());
      if (!curr_tr_op)
        continue;

      auto curr_perm = curr_tr_op->getPermute();
      auto is_last_two_dims_swapped =
          curr_perm.size() >= 2 &&
          curr_perm[curr_perm.size() - 1] == (int)(curr_perm.size() - 2) &&
          curr_perm[curr_perm.size() - 2] == (int)(curr_perm.size() - 1);

      if (next_op->getOpType() == OpType::MatMul && is_last_two_dims_swapped) {
        // 先转换为 MatmulObj *
        auto mul_op = dynamic_cast<MatmulObj *>(next_op.get());
        if (!mul_op)
          continue;

        // 区分 curr_out 是 A 还是 B, 分情况讨论
        if (out->getGuid() == mul_op->getInputs(0)->getGuid()) {
          mul_op->setTransA(!mul_op->getTransA());
        } else {
          mul_op->setTransB(!mul_op->getTransB());
        }
        // 根据测试样例, 还要把对应的 A / B 直接替换为 curr_in
        op_map[mul_op->getGuid()]->replaceInput(op->getOutput(0), in);

        op_map.erase(guid);           // curr_op
        op_map.erase(out->getGuid()); // curr_out
      }
    }
  }
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