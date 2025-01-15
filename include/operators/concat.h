#pragma once
#include "core/operator.h"

namespace infini {

/**
 * @brief Concatenate several tensors into one. All the input tensors should
 * have the same shape except for the concatenated dimension.
 *
 */
class ConcatObj : public OperatorObj {
  int dim;

public:
  /**
   * @brief Construct a new Concat object.
   *
   * @param graph The computation graph that this operator belongs to.
   * @param inputs The input tensors to be concatenated.
   * @param output Concatenated tensor.
   * @param input_dim The dimension to concatenate on.
   */
  ConcatObj(GraphObj *graph, const TensorVec &inputs, Tensor output,
            int input_dim);
  OP_CLONE(ConcatObj);

  optional<vector<Shape>> inferShape(const TensorVec &inputs) override;

  [[nodiscard]] std::string toString() const override;
  [[nodiscard]] int numInputs() const override {
    return static_cast<int>(inputs.size());
  }
  [[nodiscard]] int numOutputs() const override { return 1; }
  [[nodiscard]] int getDim() const { return dim; }
};

} // namespace infini
