#pragma once
#include "core/operator.h"

namespace infini {

/**
 * @brief Transpose the input tensor similar to numpy.transpose.
 *
 */
class TransposeObj : public OperatorObj {
public:
  /**
   * @brief Construct a new TransposeObj object.
   *
   * @param graph The graph to which this operator belongs.
   * @param input The input tensor.
   * @param output The output tensor.
   * @param permute The permutation of the dimensions.
   */
  TransposeObj(GraphObj *graph, const Tensor &input, Tensor output,
               vector<int> permute);
  OP_CLONE(TransposeObj);
  optional<vector<Shape>> inferShape(const TensorVec &inputs) override;

  [[nodiscard]] std::string toString() const override;
  [[nodiscard]] int numInputs() const override { return 1; }
  [[nodiscard]] int numOutputs() const override { return 1; }
  [[nodiscard]] std::vector<int> getPermute() const { return transposePermute; }

private:
  vector<int> transposePermute;
};

} // namespace infini
