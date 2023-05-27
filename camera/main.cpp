#include <chrono>
#include <condition_variable>
#include <iomanip>
#include <iostream>
#include <memory>

#include <string.h>
#include <config.h>
#include <camera.h>
#include <compression_pool.h>
#include <rsics_server.h>


#include <buffer_pool.h>
#include <threading.h>

#define FRAME_COUNT_RANGE 2
#define FRAME_COUNT 100
#define BYTES_PER_FRAME 1465


int main() {

  rscamera::Camera camera;
  std::thread camera_thread;
  camera_thread = std::thread([&camera, &camera_thread]() {
    set_thread_affinity(camera_thread, 0);
    camera.start();
  });

  rsics::BroadcastServer server;

  uint64_t frames_processed = 0;
  auto start = std::chrono::high_resolution_clock::now();

  server.start();

  rscamera::Pipeline<std::vector<libcamera::Span<uint8_t>>> frame_pipeline;
  rscamera::Pipeline<rscamera::CompressedObject> compression_pipeline;
  rscamera::CompressionPool compression_pool(&frame_pipeline, &compression_pipeline,
                                   camera.dimensions().stride, {0, 1, 2, 3});

  std::thread sending_thread;
  sending_thread = std::thread(
      [&server, &compression_pipeline, &frames_processed, &sending_thread]() {
        while (true) {
          rscamera::CompressedObject frame = compression_pipeline.pop();
          server.broadcast(frame.object, frame.size);
          // if (frame.size / BYTES_PER_FRAME > FRAME_COUNT + FRAME_COUNT_RANGE)
          // { 	compresser.dec_quality(); } else if (frame.size /
          // BYTES_PER_FRAME < FRAME_COUNT - FRAME_COUNT_RANGE) {
          // compresser.inc_quality();
          // }
          #if DEBUG_PRINT
          frames_processed++;
          #endif
        }
      });

  while (true) {

    rscamera::CompletedRequest *req = camera.completed_request();

    libcamera::Stream *camera_stream = camera.stream();

    libcamera::FrameBuffer *buffer = req->buffers[camera_stream];
    std::vector<libcamera::Span<uint8_t>> frame_buffer = camera.buffer(buffer);

    frame_pipeline.add(frame_buffer);

    auto current = std::chrono::high_resolution_clock::now();
    auto difference =
        std::chrono::duration_cast<std::chrono::seconds>(current - start)
            .count();

    #if DEBUG_PRINT
      uint64_t frames_per_second = frames_processed / difference;
      std::cout << frames_per_second << "\n";
    #endif

    camera.next_frame(req);
    delete req;
  }
  
  return 0;
}