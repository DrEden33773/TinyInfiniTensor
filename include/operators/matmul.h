#pragma once
#include "core/operator.h"

namespace infini {

/**
 * @brief Matrix multiplication.
 *
 */
class MatmulObj : public OperatorObj {
private:
  // InfiniTensor assumes a row-major tensor layout. `transA`=false means
  // default dims, true means A should be transposed before matmul. This is in
  // oppsite to the column-major BLAS.
  bool transA, transB;

  // Auxiliary attributes which are not a part of operator attributes.
  int m{}, n{}, k{};

public:
  /**
   * @brief Matmul operator with batch broadcast and tensor transpose
   * supports. Only one tensor with singe batch can be broadcasted due to the
   * BLAS interface restriction. Transpose indicates whether the last two
   * dimensions should be transposed before Matmul and does not affect other
   * leading dimensions.
   *
   * Matmul show how operators are defined in InfiniTensor. The constructor of
   * an operator can create output tensors for the operator or not, which
   * depends on `graph`.
   *
   * @param graph The computation graph that this operator belongs to.
   * @param A The input tensor.
   * @param B The input tensor.
   * @param C C is the output of Matmul. If outputs are going to be created in
   * the constructor, C should be an empty Ref.
   * @param transA If matrix A should be transposed when computing.
   * @param transB If matrix B should be transposed when computing.
   */
  MatmulObj(GraphObj *graph, Tensor A, Tensor B, Tensor C, bool transA = false,
            bool transB = false);
  OP_CLONE(MatmulObj);

  [[nodiscard]] std::string toString() const override;
  optional<vector<Shape>> inferShape(const TensorVec &inputs) override;

  [[nodiscard]] int numInputs() const override { return (int)inputs.size(); }
  [[nodiscard]] int numOutputs() const override { return 1; }

  [[nodiscard]] bool getTransA() const { return transA; }
  [[nodiscard]] bool getTransB() const { return transB; }
  void setTransA(bool transA) { this->transA = transA; }
  void setTransB(bool transB) { this->transB = transB; }
  [[nodiscard]] int getM() const { return m; }
  [[nodiscard]] int getN() const { return n; }
  [[nodiscard]] int getK() const { return k; }
};

} // namespace infini