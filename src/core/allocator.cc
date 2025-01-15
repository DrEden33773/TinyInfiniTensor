#include "core/allocator.h"
#include "fmt/base.h"
#include "fmt/core.h"
#include "fmt/format.h"
#include "utils/print.hpp"
#include "gmock/gmock.h"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <utility>

#define MERGE_ADJ_LEFT_FREE_BLOCK

namespace infini {

Allocator::Allocator(Runtime runtime)
    : runtime(std::move(runtime)), used(0), peak(0),
      alignment(sizeof(uint64_t)), ptr(nullptr) {
  // 'alignment' defaults to sizeof(uint64_t), because it is the length of
  // the longest data type currently supported by the DataType field of
  // the tensor
}

Allocator::~Allocator() {
  if (this->ptr != nullptr) {
    runtime->dealloc(this->ptr);
  }
}

size_t Allocator::alloc(size_t size) {
  IT_ASSERT(this->ptr == nullptr);
  size = this->getAlignedSize(size);

  for (auto it = available.begin(); it != available.end(); it++) {
    if (it->second < size) {
      continue;
    }

    auto offset = it->first;
    allocated[offset] = size;

    used += size;
    peak = std::max(peak, used);

    auto remaining = it->second - size;
    if (remaining > 0) {
      available[offset + size] = remaining;
    }

    available.erase(it);
    return offset;
  }

  auto msg =
      fmt::format("[PANIC]: no available memory block of size `{}`", size);
  testing::Throw(msg);

  return 0;
}

void Allocator::free(size_t addr, size_t size) {
  IT_ASSERT(this->ptr == nullptr);
  size = getAlignedSize(size);

  auto it = allocated.find(addr);

  if (it == allocated.end()) {
    auto msg = fmt::format("[PANIC]: offset[{}] was not allocated", addr);
    testing::Throw(msg);
  }

  if (it->second < size) {
    auto msg = fmt::format(
        "[PANIC]: offset[{}] allocated size `{}` < expected size `{}` to free",
        addr, it->second, size);
    testing::Throw(msg);
  }

  if (it->second != size) {
    println(
        "[WARN]: offset[{}] allocated size `{}` > expected size `{}` to free",
        addr, it->second, size);
  }

  // free the block
  used -= size;
  allocated.erase(it);
  available[addr] = size;

  // merge adjacent free block on the right
  auto right_it = available.find(addr + size);
  if (right_it != available.end()) {
    size += right_it->second;
    available.erase(right_it);
    available[addr] = size;
  }

// then merge adjacent free block on the left
#ifdef MERGE_ADJ_LEFT_FREE_BLOCK
  auto left_it = available.lower_bound(addr); // the first <=, instead of <
  if (left_it != available.begin()) {
    auto prev_it = std::prev(left_it); // that's why we need the `prev` iterator
    if (prev_it->first + prev_it->second == addr) {
      addr = prev_it->first;
      size += prev_it->second;
      available.erase(prev_it);
      available[addr] = size;
    }
  }
#endif
}

void *Allocator::getPtr() {
  if (this->ptr == nullptr) {
    this->ptr = runtime->alloc(this->peak);
    println("Allocator really alloc `{}` with `{}` bytes", this->ptr,
            this->peak);
  }
  return this->ptr;
}

size_t Allocator::getAlignedSize(size_t size) const {
  return ((size - 1) / this->alignment + 1) * this->alignment;
}

void Allocator::info() {
  println("Used memory: {}, peak memory: {}", this->used, this->peak);
}

} // namespace infini
