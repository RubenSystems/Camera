#pragma once

#include <libcamera/camera.h>
#include <libcamera/camera_manager.h>
#include <libcamera/control_ids.h>
#include <libcamera/controls.h>
#include <libcamera/formats.h>
#include <libcamera/framebuffer_allocator.h>
#include <libcamera/property_ids.h>

namespace rscamera {

struct CompletedRequest {

  CompletedRequest(unsigned int count, libcamera::Request *r)
      : counter(count), buffers(r->buffers()), metadata(r->metadata()),
        request(r) {
    r->reuse();
  }
  unsigned int counter;
  libcamera::Request::BufferMap buffers;
  libcamera::ControlList metadata;
  libcamera::Request *request;
  float framerate;
};

} // namespace rscamera