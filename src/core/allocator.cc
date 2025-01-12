#include "core/allocator.h"
#include "utils/exception.h"
#include <cstdio>
#include <utility>

namespace infini {
Allocator::Allocator(Runtime runtime) : runtime(runtime) {
  used = 0;
  peak = 0;
  ptr = nullptr;

  // 'alignment' defaults to sizeof(uint64_t), because it is the length of
  // the longest data type currently supported by the DataType field of
  // the tensor
  alignment = sizeof(uint64_t);
}

Allocator::~Allocator() {
  if (this->ptr != nullptr) {
    runtime->dealloc(this->ptr);
  }
}

size_t Allocator::alloc(size_t size) {
  IT_ASSERT(this->ptr == nullptr);
  size = this->getAlignedSize(size);

  // Find a free block that is large enough
  for (auto it = free_blocks.begin(); it != free_blocks.end(); ++it) {
    if (it->second >= size) {
      usize offset = it->first;
      usize remaining = it->second - size;

      // Remove the block from free_blocks
      free_blocks.erase(it);

      // If there is remaining space, add it back to free_blocks
      if (remaining > 0) {
        free_blocks[offset + size] = remaining;
      }

      used += size;
      if (used > peak) {
        peak = used;
      }

      return offset;
    }
  }

  // If no suitable block is found, initialize a new block
  if (free_blocks.empty()) {
    free_blocks[0] = (1ULL << 30);
  }

  return 0;
}

void Allocator::free(size_t addr, size_t size) {
  IT_ASSERT(this->ptr == nullptr);
  size = getAlignedSize(size);

  // =================================== 作业
  // ===================================
  // TODO: 设计一个算法来回收内存
  // =================================== 作业
  // ===================================

  auto it = free_blocks.find(addr);

  if (it == free_blocks.end()) {
    // cannot find the block, double free error
    std::string msg = "offset[" + std::to_string(addr) + "]: Not allocated";
    throw Exception(msg);
  }

  if (it->second < size) {
    std::string msg = "offset[" + std::to_string(addr) + "] allocated size `" +
                      std::to_string(it->second) +
                      "` is less than expected free size `" +
                      std::to_string(size) + "`";
    throw Exception(msg);
  }

  if (it->second != size) {
    printf(
        "[Warning]: offset[%zu] allocated size `%zu` is larger than expected "
        "free size `%zu`\n",
        addr, it->second, size);
    printf("\t>>> Default solution = free the whole block\n");
  }

  // free the block
  used -= size;
  free_blocks.erase(it);
}

void *Allocator::getPtr() {
  if (this->ptr == nullptr) {
    this->ptr = runtime->alloc(this->peak);
    printf("Allocator really alloc: %p %zu bytes\n", this->ptr, peak);
  }
  return this->ptr;
}

size_t Allocator::getAlignedSize(size_t size) {
  return ((size - 1) / this->alignment + 1) * this->alignment;
}

void Allocator::info() {
  std::cout << "Used memory: " << this->used << ", peak memory: " << this->peak
            << std::endl;
}
} // namespace infini
