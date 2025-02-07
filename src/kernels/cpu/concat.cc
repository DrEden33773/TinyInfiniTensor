#include "operators/concat.h"
#include "core/kernel.h"

namespace infini {

class NaiveConcat : public CpuKernelWithoutConfig {
  template <typename T>
  void doCompute(const Operator &_op, const RuntimeObj *context) const {
    auto op = as<ConcatObj>(_op);
    auto inputs = op->getInputs();
    auto outputs = op->getOutputs();
    auto dim = op->getDim();
    const auto &output = outputs[0];
    std::vector<Shape> iDims;
    for (const auto &input : inputs) {
      iDims.emplace_back(input->getDims());
    }
    const auto &outDim = output->getDims();
    size_t blockOffsetInner = 1;
    for (size_t i = outDim.size() - 1; i > static_cast<size_t>(dim); --i) {
      blockOffsetInner *= outDim[i];
    }
    size_t blockOffset = outDim[dim] * blockOffsetInner;
    for (size_t i = 0; i < inputs.size(); ++i) {
      const auto &input = inputs[i];
      auto dimOffset = 0;
      const auto &iDim = iDims[i];
      for (size_t j = 0; j < i; ++j) {
        dimOffset += iDims[j][dim];
      }
      size_t localBlockOffset = 1;
      for (size_t k = iDim.size() - 1;
           k >= static_cast<size_t>(dim) && k != static_cast<size_t>(-1); --k) {
        localBlockOffset *= iDim[k];
      }
      auto innerOffset = blockOffsetInner * dimOffset;
      auto inSize = input->size();
      auto *inPtr = input->getRawDataPtr<T *>();
      auto *outPtr = output->getRawDataPtr<T *>();
#pragma omp parallel for
      for (size_t iOffset = 0; iOffset < inSize; ++iOffset) {
        auto oOffset = (iOffset % localBlockOffset) + innerOffset +
                       (iOffset / localBlockOffset * blockOffset);
        outPtr[oOffset] = inPtr[iOffset];
      }
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

REGISTER_KERNEL(Device::CPU, OpType::Concat, NaiveConcat, "ConcatNaive_CPU");

} // namespace infini
