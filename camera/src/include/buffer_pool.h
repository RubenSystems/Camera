#pragma once

#include <stdint.h>
#include <stdlib.h>

namespace rscamera {

class BufferPool {

public:
  BufferPool(uint8_t count, size_t size);

  BufferPool(const BufferPool &) = delete;
  BufferPool *operator=(const BufferPool &) = delete;

  ~BufferPool();

public:
  uint8_t *buffer(uint8_t idx);

  size_t buffer_size();

private:
  // Try make congiguious
  uint8_t **buffers_;
  uint8_t buffer_count_;
  size_t buffer_size_;
};

} // namespace rscamera