#pragma once
#include "core/runtime.h"
#ifdef BUILD_TEST
#include "gtest/gtest.h"
#endif
#include <cstddef>
#include <map>

namespace infini {

class Allocator {
private:
  // Default Value = 16MB
  static constexpr size_t MAX_SIZE = 512ULL << 20;

  Runtime runtime;

  // used memory size
  size_t used;

  // peak of `used` history value
  size_t peak;

  size_t alignment;

  // pointer to the memory actually allocated
  void *ptr;

  /*
  `key`: start address
  `value`: size
   */
  std::map<size_t, size_t> allocated;

  /*
  `key`: start address
  `value`: size
   */
  std::map<size_t, size_t> available{{0, MAX_SIZE}};

public:
  Allocator(Runtime runtime);

  virtual ~Allocator();

  // function: simulate memory allocation
  // arguments：
  //     size: size of memory block to be allocated
  // return: head address offset of the allocated memory block
  size_t alloc(size_t size);

  // function: simulate memory free
  // arguments:
  //     addr: head address offset of memory block to be free
  //     size: size of memory block to be freed
  void free(size_t addr, size_t size);

  // function: perform actual memory allocation
  // return: pointer to the head address of the allocated memory
  void *getPtr();

  void info();

private:
  // function: memory alignment, rounded up
  // return: size of the aligned memory block
  [[nodiscard]] size_t getAlignedSize(size_t size) const;
};
} // namespace infini
