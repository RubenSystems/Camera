#pragma once

#include "pipeline.h"
#include <vector>

#include <camera.h>
#include <compression.h>

namespace rscamera {
class CompressionPool {
public:
  CompressionPool(
      rscamera::Pipeline<std::vector<libcamera::Span<uint8_t>>>
          *ingress_pipeline,
      rscamera::Pipeline<rscamera::CompressedObject> *egress_pipeline,
      int stride, const std::vector<int> &core_mapping);

private:
  std::vector<std::thread> threads_;
};

} // namespace rscamera