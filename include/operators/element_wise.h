#pragma once
#include "core/operator.h"

namespace infini {

/**
 * @brief Base class of **binary** element-wise operators.
 * Unary operators like activations are not the derived classes of
 * ElementWiseObj.
 *
 */
class ElementWiseObj : public OperatorObj {
public:
  /**
   * @brief Construct a new ElementWise object
   *
   * @param type Operator type.
   * @param graph The computation graph that this operator belongs to.
   * @param input0 The first input tensor.
   * @param input1 The second input tensor.
   * @param output The output tensor.
   */
  ElementWiseObj(OpType type, GraphObj *graph, Tensor input0, Tensor input1,
                 Tensor output);
  optional<vector<Shape>> inferShape(const TensorVec &inputs) override;

  std::string toString() const override;
  int numInputs() const override { return 2; }
  int numOutputs() const override { return 1; }
};

#define DEFINE_ELEMENT_WISE_OBJ(prefix, type)                                  \
  class prefix##Obj : public ElementWiseObj {                                  \
  public:                                                                      \
    prefix##Obj(GraphObj *graph, Tensor input0, Tensor input1, Tensor output)  \
        : ElementWiseObj(type, graph, std::move(input0), std::move(input1),    \
                         std::move(output)) {}                                 \
    OP_CLONE(prefix##Obj);                                                     \
  };

DEFINE_ELEMENT_WISE_OBJ(Add, OpType::Add)
DEFINE_ELEMENT_WISE_OBJ(Sub, OpType::Sub)
DEFINE_ELEMENT_WISE_OBJ(Mul, OpType::Mul)
DEFINE_ELEMENT_WISE_OBJ(Div, OpType::Div)

}; // namespace infini
