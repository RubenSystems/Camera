

#include <compression_pool.h>
#include <threading.h>
using namespace rscamera;

CompressionPool::CompressionPool(
    rscamera::Pipeline<std::vector<libcamera::Span<uint8_t>>> *ingress_pipeline,
    rscamera::Pipeline<rscamera::CompressedObject> *egress_pipeline, int stride,
    const std::vector<int> &core_mapping)
    : threads_(core_mapping.size()) {

  for (int i = 0; i < (int)core_mapping.size(); i++) {
    threads_[i] = std::thread([ingress_pipeline, egress_pipeline, stride]() {
      rscamera::Compresser compresser_(CAMERA_WIDTH, CAMERA_HEIGHT, stride,
                                       egress_pipeline);
      while (true) {
        std::vector<libcamera::Span<uint8_t>> frame = ingress_pipeline->pop();
        uint8_t *frame_memory = frame[0].data();
        compresser_.compress(frame_memory);
      }
    });
    set_thread_affinity(threads_[i], core_mapping[i]);
  }
}