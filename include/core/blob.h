#pragma once
#include <utility>

#include "core/ref.h"

namespace infini {

class RuntimeObj;
using Runtime = Ref<RuntimeObj>;

class BlobObj {
  Runtime runtime;
  void *ptr;

public:
  BlobObj(Runtime runtime, void *ptr) : runtime(std::move(runtime)), ptr(ptr) {}
  BlobObj(BlobObj &other) = delete;
  BlobObj &operator=(BlobObj const &) = delete;
  ~BlobObj() = default;

  template <typename T> T getPtr() const { return static_cast<T>(ptr); }
};

} // namespace infini
