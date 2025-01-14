#include "operators/concat.h"

#include "utils/operator_utils.h"
#include <algorithm>
#include <cstddef>
#include <optional>
#include <utility>

namespace infini {
ConcatObj::ConcatObj(GraphObj *graph, TensorVec inputs, Tensor output, int _dim)
    : OperatorObj(OpType::Concat, inputs, {std::move(output)}) {
  int rank = (int)inputs[0]->getRank();
  dim = get_real_axis(_dim, rank);
  IT_ASSERT(checkValid(graph));
}

optional<vector<Shape>> ConcatObj::inferShape(const TensorVec &inputs) {
  if (inputs.empty()) {
    return std::nullopt;
  }

  // check rank
  auto rank = inputs[0]->getRank();
  auto same_rank =
      std::all_of(inputs.begin() + 1, inputs.end(), [&](const Tensor &input) {
        return input->getRank() == rank;
      });
  if (!same_rank) {
    return std::nullopt;
  }

  // axis has been checked in the constructor => dim

  Shape output_shape = inputs[0]->getDims();
  int int_rank = (int)rank;

  for (auto it = inputs.begin() + 1; it != inputs.end(); ++it) {
    auto input_shape = (*it)->getDims();
    // check shape
    for (int i = 1; i < int_rank; ++i) {
      if (i != dim && output_shape[i] != input_shape[i]) {
        return std::nullopt;
      }
    }
    output_shape[dim] += input_shape[dim];
  }

  return {{output_shape}};
}

std::string ConcatObj::toString() const {
  std::ostringstream os;
  os << "Concat[" << getGuid() << "]";
  os << "(";
  for (const auto &input : inputs)
    os << vecToString(input->getDims()) << ", ";
  os << "dim=" << dim << ", ";
  os << "input=";
  for (const auto &input : inputs)
    os << input->getGuid() << ", ";
  os << "output=" << outputs[0]->getGuid() << ")";
  return os.str();
}

} // namespace infini
