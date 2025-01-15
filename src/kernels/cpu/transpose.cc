#include "operators/transpose.h"
#include "core/kernel.h"

namespace infini {

inline Shape idx2Pos(const Shape &shape, size_t idx) {
  Shape pos = Shape(shape.size(), 0);
  auto rest = idx;
  auto curDimId = shape.size() - 1;
  while (rest > 0) {
    pos[curDimId] = static_cast<int>(rest) % shape[curDimId];
    rest /= shape[curDimId];
    curDimId--;
  }
  return pos;
}

class NaiveTranspose : public CpuKernelWithoutConfig {
  template <typename T>
  void doCompute(const Operator &_op, const RuntimeObj *context) const {
    auto op = as<TransposeObj>(_op);
    auto inputs = op->getInputs();
    auto outputs = op->getOutputs();
    const auto &inDim = inputs[0]->getDims();
    const auto &perm = op->getPermute();

    size_t inSize = inputs[0]->size();
    auto *inPtr = inputs[0]->getRawDataPtr<T *>();
    auto *outPtr = outputs[0]->getRawDataPtr<T *>();
#pragma omp parallel for
    for (size_t inIdx = 0; inIdx < inSize; ++inIdx) {
      auto posInput = idx2Pos(inDim, inIdx);
      int outIdx = 0;
      for (int j : perm) {
        outIdx = outIdx * inDim[j] + posInput[j];
      }
      outPtr[outIdx] = inPtr[inIdx];
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

REGISTER_KERNEL(Device::CPU, OpType::Transpose, NaiveTranspose,
                "TransposeNaive_CPU");

} // namespace infini
