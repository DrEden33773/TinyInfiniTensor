#include "operators/transpose.h"

#include <cstddef>
#include <utility>

namespace infini {
TransposeObj::TransposeObj(GraphObj *graph, const Tensor &input, Tensor output,
                           vector<int> permute)
    : OperatorObj(OpType::Transpose, {input}, {std::move(output)}) {
  auto rank = input->getRank();

  if (permute.empty()) {
    for (size_t i = 0; i < rank; ++i) {
      transposePermute[i] = (int)i;
    }
  } else {
    IT_ASSERT(rank == permute.size());
    transposePermute = std::move(permute);
  }
  IT_ASSERT(checkValid(graph));
}

optional<vector<Shape>> TransposeObj::inferShape(const TensorVec &inputs) {
  if (inputs.size() != 1) {
    return std::nullopt;
  }

  const auto &input_tensor = inputs[0];

  auto input_shape = input_tensor->getDims();
  auto output_shape = input_shape;

  auto rank = input_tensor->getRank();
  if (rank == transposePermute.size()) {
    for (size_t i = 0; i < rank; ++i) {
      output_shape[i] = input_shape[transposePermute[i]];
    }
    return vector<Shape>{output_shape};
  }

  return std::nullopt;
}

std::string TransposeObj::toString() const {
  std::ostringstream os;
  os << type.toString() << "[" << getGuid() << "]";
  os << "(";
  os << vecToString(inputs[0]->getDims()) << ", ";
  os << "input=" << inputs[0]->getGuid() << ", ";
  os << "output=" << outputs[0]->getGuid() << ")";
  return os.str();
}
}; // namespace infini
