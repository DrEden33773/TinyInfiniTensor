#include "core/runtime.h"
#include "core/graph.h"
#include "core/kernel.h"
#include <cstring>

namespace infini {

void NativeCpuRuntimeObj::run(const Graph &graph) const {
  const auto &kernelRegistry = KernelRegistry::getInstance();

  for (const auto &op : graph->getOperators()) {
    auto kernelAttrs = KernelAttrs{device, op->getOpType().underlying()};
    Kernel *kernel = kernelRegistry.getKernel(kernelAttrs);
    kernel->compute(op, this);
  }
}

string NativeCpuRuntimeObj::toString() const { return "CPU"; }

void NativeCpuRuntimeObj::dealloc(void *ptr) { free(ptr); }

void *NativeCpuRuntimeObj::alloc(size_t size) {
  return calloc((size + sizeof(uint64_t) - 1) / sizeof(uint64_t),
                sizeof(uint64_t));
}

} // namespace infini
