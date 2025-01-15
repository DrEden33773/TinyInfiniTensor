#include "utils/operator_utils.h"
#include "core/runtime.h"

namespace infini {

Shape infer_broadcast(const Shape &A, const Shape &B) {
  // 1. 两个形状的维度数取最大值
  size_t max_rank = std::max(A.size(), B.size());
  Shape broadcast_shape(max_rank, 1);

  // 2. 从后往前遍历，对每一维进行广播
  size_t i = A.size();
  size_t j = B.size();
  size_t k = max_rank;

  while (k > 0) {
    size_t a = i > 0 ? A[--i] : 1;
    size_t b = j > 0 ? B[--j] : 1;
    // 3. 如果两个维度不相等，也不是 1，则无法广播
    if (a != b && a != 1 && b != 1) {
      return {};
    }

    // 4. 取两个维度的最大值
    broadcast_shape[--k] = static_cast<int>(std::max(a, b));
  }

  return broadcast_shape;
}

int get_real_axis(const int &axis, const int &rank) {
  IT_ASSERT(rank >= 1);
  IT_ASSERT(axis >= -rank && axis <= (rank - 1));
  int newAxis{};
  if (axis < 0) {
    newAxis = rank + axis;
  } else {
    newAxis = axis;
  }
  return newAxis;
}

Shape locate_index(size_t inputN, const Shape &shape) {
  Shape ans(shape.size());
  auto i = ans.rbegin();
  auto j = shape.rbegin();
  auto ej = shape.rend();
  while (j != ej) {
    auto div = std::div(static_cast<int>(inputN), *j++);
    *i++ = div.rem;
    inputN = div.quot;
  }
  return ans;
}

size_t delocate_index(const Shape &shapeIndex, const Shape &shape,
                      const Shape &stride) {
  size_t ans = 0;
  Shape index(shapeIndex.size());
  IT_ASSERT(shapeIndex.size() == shape.size());
  IT_ASSERT(shape.size() == stride.size());
  for (size_t i = 0; i < shape.size(); ++i) {
    index[i] = shapeIndex[i] % shape[i];
    ans += static_cast<long>(index[i]) * stride[i];
  }
  return ans;
}

std::string device_to_str(Device device) {
  // std::string deviceStr;
  switch (device) {
  case Device::CPU:
    return "CPU";
  default:
    IT_TODO_HALT();
  }
}

std::string get_kernel_attrs_str(const KernelAttrs &kernelAttrs) {
  std::string deviceStr = device_to_str(std::get<0>(kernelAttrs));
  std::string opStr = OpType(std::get<1>(kernelAttrs)).toString();
  return deviceStr + ", " + opStr;
}

} // namespace infini
