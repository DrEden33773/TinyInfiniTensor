#include "operators/element_wise.h"
#include "core/kernel.h"
#include "utils/operator_utils.h"

namespace infini {

class NativeElementWise : public CpuKernelWithoutConfig {
  template <typename T> static T addCompute(T val0, T val1) {
    return val0 + val1;
  }

  template <typename T> static T subCompute(T val0, T val1) {
    return val0 - val1;
  }

  template <typename T> static T mulCompute(T val0, T val1) {
    return val0 * val1;
  }

  template <typename T> static T divCompute(T val0, T val1) {
    return static_cast<T>(val0 / val1);
  }

  template <typename T>
  void doCompute(const Operator &_op, const RuntimeObj *context) const {
    auto op = as<ElementWiseObj>(_op);
    T *inptr0 = op->getInputs(0)->getRawDataPtr<T *>();
    T *inptr1 = op->getInputs(1)->getRawDataPtr<T *>();
    T *outptr = op->getOutput()->getRawDataPtr<T *>();

    auto shapeA = op->getInputs(0)->getDims();
    auto shapeB = op->getInputs(1)->getDims();
    auto shapeC = op->getOutput()->getDims();
    auto rank = op->getOutput()->getRank();
    Shape a(rank, 1);
    Shape b(rank, 1);
    std::copy(shapeA.begin(), shapeA.end(),
              a.begin() + static_cast<long>(rank - shapeA.size()));
    std::copy(shapeB.begin(), shapeB.end(),
              b.begin() + static_cast<long>(rank - shapeB.size()));
    auto getStride = [&](const Shape &shape) {
      int p = 1;
      Shape stride(rank);
      for (auto i = rank; i > 0; --i) {
        stride[i - 1] = p;
        p = p * shape[i - 1];
      }
      return stride;
    };
    Shape strideA = getStride(a);
    Shape strideB = getStride(b);

    auto n = op->getOutput()->size();
    T (*_doCompute)(T val0, T val1){nullptr};
    switch (op->getOpType().underlying()) {
    case OpType::Add:
      _doCompute = addCompute<T>;
      break;
    case OpType::Sub:
      _doCompute = subCompute<T>;
      break;
    case OpType::Mul:
      _doCompute = mulCompute<T>;
      break;
    case OpType::Div:
      _doCompute = divCompute<T>;
      break;
    default:
      IT_TODO_HALT();
    }

    for (size_t i = 0; i < n; ++i) {
      auto shapeIndexC = locate_index(i, shapeC);
      auto indexA = delocate_index(shapeIndexC, a, strideA);
      auto indexB = delocate_index(shapeIndexC, b, strideB);
      outptr[i] = _doCompute(inptr0[indexA], inptr1[indexB]);
    }
  }

  void compute(const Operator &_op, const RuntimeObj *context) const override {
#define CASE(N)                                                                \
  case N:                                                                      \
    doCompute<DT<N>::t>(_op, context)

    switch (_op->getDType().getIndex()) {
      CASE(1); // DataType::Float32
      break;
      CASE(12); // DataType::UInt32
      break;
    default:
      IT_TODO_HALT();
    }
  }
};

REGISTER_KERNEL(Device::CPU, OpType::Add, NativeElementWise, "addNaive_CPU");
REGISTER_KERNEL(Device::CPU, OpType::Sub, NativeElementWise, "subNaive_CPU");
REGISTER_KERNEL(Device::CPU, OpType::Mul, NativeElementWise, "mulNaive_CPU");
REGISTER_KERNEL(Device::CPU, OpType::Div, NativeElementWise, "divNaive_CPU");

}; // namespace infini
