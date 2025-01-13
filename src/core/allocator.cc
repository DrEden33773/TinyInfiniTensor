#include "core/allocator.h"
#include "fmt/base.h"
#include "fmt/core.h"
#include "utils/exception.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>

namespace infini {
static constexpr const char *SPLITTER =
    "----------------------------------------";

Allocator::Allocator(Runtime runtime) : runtime(std::move(runtime)) {
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

  // If no suitable block is found, initialize a new block first
  if (free_blocks.empty()) {
    free_blocks[0] = (1ULL << 20);
  }

  // Find a free block that is large enough
  for (auto it = free_blocks.begin(); it != free_blocks.end(); ++it) {
    if (it->second >= size) {
      size_t offset = it->first;
      size_t remaining = it->second - size;

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

  return 0;
}

void Allocator::free(size_t addr, size_t size) {
  IT_ASSERT(this->ptr == nullptr);
  size = getAlignedSize(size);

  auto it = free_blocks.find(addr);

  if (it == free_blocks.end()) {
    fmt::println(SPLITTER);
    fmt::println("[PANIC!]: offset[{}] was not allocated", addr);
    fmt::println(SPLITTER);
    exit(-1);
  }

  if (it->second < size) {
    fmt::println(SPLITTER);
    fmt::println(
        "[PANIC!]: offset[{}] allocated size `{}` < expected size `{}` "
        "to be freed",
        addr, it->second, size);
    fmt::println(SPLITTER);
    exit(-1);
  }

  if (it->second != size) {
    fmt::println(SPLITTER);
    fmt::println("[WARNING?]: offset[{}] allocated size `{}` > expected size "
                 "`{}` to be freed",
                 addr, it->second, size);
    fmt::println(SPLITTER);
  }

  // free the block
  used -= size;
  free_blocks.erase(it);
}

void *Allocator::getPtr() {
  if (this->ptr == nullptr) {
    this->ptr = runtime->alloc(this->peak);
    fmt::println("Allocator really alloc: {} {} bytes", this->ptr, this->peak);
  }
  return this->ptr;
}

size_t Allocator::getAlignedSize(size_t size) {
  return ((size - 1) / this->alignment + 1) * this->alignment;
}

void Allocator::info() {
  fmt::print("Used memory: {}, peak memory: {}\n", this->used, this->peak);
}
} // namespace infini
