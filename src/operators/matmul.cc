#include "operators/matmul.h"
#include "core/tensor.h"
#include "utils/operator_utils.h"
#include <iterator>
#include <utility>

namespace infini {

MatmulObj::MatmulObj(GraphObj *graph, Tensor A, Tensor B, Tensor C, bool transA,
                     bool transB)
    : OperatorObj(OpType::MatMul, TensorVec{std::move(A), std::move(B)},
                  {std::move(C)}),
      transA(transA), transB(transB) {
  IT_ASSERT(checkValid(graph));
}

string MatmulObj::toString() const {
  std::ostringstream os;
  os << "Matmul([" << (transA ? "A^T" : "A") << "," << (transB ? "B^T" : "B]")
     << ", A=" << inputs[0]->getGuid() << ", B=" << inputs[1]->getGuid()
     << ", C=" << outputs[0]->getGuid() << ", mnk=[" << m << "," << n << ","
     << k << "])";
  return os.str();
}

optional<vector<Shape>> MatmulObj::inferShape(const TensorVec &inputs) {
  if (inputs.size() != 2) {
    return std::nullopt;
  }

  const auto &A = inputs[0];
  const auto &B = inputs[1];
  auto A_rank = A->getRank();
  auto A_shape = A->getDims();
  auto B_rank = B->getRank();
  auto B_shape = B->getDims();

  if (A->getRank() < 2 || B->getRank() < 2) {
    return std::nullopt;
  }

  // 1. Deal with `A, B`'s last 2 dims
  auto checked_A_dim_len = transA ? A_shape[A_rank - 2] : A_shape[A_rank - 1];
  auto checked_B_dim_len = transB ? B_shape[B_rank - 1] : B_shape[B_rank - 2];
  if (checked_A_dim_len != checked_B_dim_len) {
    return std::nullopt;
  }
  m = transA ? A_shape[A_rank - 1] : A_shape[A_rank - 2];
  n = checked_A_dim_len;
  k = transB ? B_shape[B_rank - 2] : B_shape[B_rank - 1];

  Shape part_b{m, k};

  if (A_rank == 2 && B_rank == 2) {
    return {{part_b}};
  }

  // 2. Broadcast the rest of the `A, B`'s previous dims
  Shape A_part_a(A_shape.begin(), A_shape.end() - 2);
  Shape B_part_a(B_shape.begin(), B_shape.end() - 2);
  Shape part_a = infer_broadcast(A_part_a, B_part_a);

  // 3. Concatenate
  part_a.reserve(part_a.size() + part_b.size());
  part_a.insert(part_a.end(), std::make_move_iterator(part_b.begin()),
                std::make_move_iterator(part_b.end()));
  return {{part_a}};
}

} // namespace infini