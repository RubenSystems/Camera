
#include <camera.h>
#include <sys/mman.h>

#include <completed_request.h>

using rscamera::Camera;

/*
        Initalisers
*/

Camera::Camera() : controls_() {
  manager_ = new libcamera::CameraManager();
  init_camera();
  if (camera_ == nullptr) {
    throw std::runtime_error("[CAMERA] - no camera");
  }
  camera_->acquire();
  configure_camera();
  create_buffer_allocator();
  configure_requests();
  get_dimensions();

  camera_->requestCompleted.connect(this, &rscamera::Camera::request_complete);
}

Camera::~Camera() {
  camera_->stop();
  // free alloc
  delete allocator_;

  camera_->release();
  camera_.reset();

  manager_->stop();
  delete manager_;
  manager_ = nullptr;
}

/*
        Publics
*/
void Camera::start() {
  camera_->start(&controls_);
  for (std::unique_ptr<libcamera::Request> &request : requests_)
    camera_->queueRequest(request.get());
}

libcamera::Stream *Camera::stream() { return stream_; }

rscamera::Dimensions Camera::dimensions() { return dimensions_; }

rscamera::CompletedRequest *Camera::completed_request() { return queue_.pop(); }

bool Camera::has_buffer(libcamera::FrameBuffer *pointer) {
  return mapped_buffers_.find(pointer) != mapped_buffers_.end();
}

std::vector<libcamera::Span<uint8_t>>
Camera::buffer(libcamera::FrameBuffer *pointer) {
  return mapped_buffers_[pointer];
}

void Camera::next_frame(rscamera::CompletedRequest *req) {
  libcamera::Request *request = req->request;
  libcamera::Request::BufferMap buffers = std::move(req->buffers);

  for (auto const &buffer : buffers) {
    if (request->addBuffer(buffer.first, buffer.second) < 0)
      throw std::runtime_error("[BUFFER] - could not readd buffer to req");
  }

  request->reuse(libcamera::Request::ReuseBuffers);
  camera_->queueRequest(request);
}

/*
        Privats
*/
void Camera::init_camera() {
  int ret = manager_->start();
  for (auto &i : manager_->cameras()) {
    camera_ = i;
    break;
  }
}

void Camera::configure_camera() {

  config_ =
      camera_->generateConfiguration({libcamera::StreamRole::VideoRecording});

  stream_config_ = &(config_->at(0));

  stream_config_->size.width = CAMERA_WIDTH;
  stream_config_->size.height = CAMERA_HEIGHT;
  stream_config_->bufferCount = CAMERA_BUFFERS;
  stream_config_->pixelFormat = libcamera::formats::YUV420;

  config_->validate();
  camera_->configure(config_.get());

  // int64_t frame_time = 1000000 / CAMERA_FPS;

  // controls_.set(libcamera::controls::ExposureTime, 0.0);
  // controls_.set(libcamera::controls::Brightness, 0.2);
  // controls_.set(libcamera::controls::AeExposureMode,
  // libcamera::controls::ExposureNormal);
  controls_.set(libcamera::controls::AwbMode, libcamera::controls::AwbAuto);
  controls_.set(libcamera::controls::AeMeteringMode,
                libcamera::controls::MeteringMatrix);
  controls_.set(libcamera::controls::AfMode,
                libcamera::controls::AfModeEnum::AfModeContinuous);
  // controls_.set(libcamera::controls::AfTrigger,
  // libcamera::controls::AfTriggerEnum::AfTriggerStart);

  set_idle();
}

void Camera::set_idle() { set_framerate(CAMERA_FPS_IDLE); }

void Camera::set_streaming() { set_framerate(CAMERA_FPS_STREAM); }

void Camera::get_dimensions() {
  libcamera::StreamConfiguration const &cfg = stream_->configuration();
  Dimensions ret = {stream_config_->size.width, stream_config_->size.height,
                    stream_config_->stride};
  dimensions_ = ret;
}

void Camera::set_framerate(int fps) {
  int64_t frame_time = 1000000 / fps;
  controls_.set(libcamera::controls::FrameDurationLimits,
                libcamera::Span<const int64_t, 2>({frame_time, frame_time}));
}

void Camera::create_buffer_allocator() {
  allocator_ = new libcamera::FrameBufferAllocator(camera_);

  for (libcamera::StreamConfiguration &cfg : *config_) {
    if (allocator_->allocate(cfg.stream()) < 0) {
      throw std::runtime_error("[CAMERA] - can't allocate buffers");
    }
    for (const std::unique_ptr<libcamera::FrameBuffer> &buffer :
         allocator_->buffers(cfg.stream())) {
      size_t buffer_size = 0;
      for (unsigned i = 0; i < buffer->planes().size(); i++) {
        const libcamera::FrameBuffer::Plane &plane = buffer->planes()[i];
        buffer_size += plane.length;
        if (i == buffer->planes().size() - 1 ||
            plane.fd.get() != buffer->planes()[i + 1].fd.get()) {
          void *memory = mmap(NULL, buffer_size, PROT_READ | PROT_WRITE,
                              MAP_SHARED, plane.fd.get(), 0);
          mapped_buffers_[buffer.get()].push_back(libcamera::Span<uint8_t>(
              static_cast<uint8_t *>(memory), buffer_size));
          buffer_size = 0;
        }
      }
    }
  }
}

void Camera::configure_requests() {
  stream_ = stream_config_->stream();
  const std::vector<std::unique_ptr<libcamera::FrameBuffer>> &buffers =
      allocator_->buffers(stream_);

  for (unsigned int i = 0; i < buffers.size(); ++i) {
    std::unique_ptr<libcamera::Request> request = camera_->createRequest();
    if (!request) {
      throw std::runtime_error("[CAMERA] - cannot make request");
    }

    libcamera::FrameBuffer *buffer = buffers[i].get();
    int ret = request->addBuffer(stream_, buffer);
    if (ret < 0) {
      throw std::runtime_error("[CAMERA] - cannot set buffer");
    }

    // libcamera::ControlList &controls = request->controls();
    // controls.set(libcamera::controls::Brightness, 0.5);
    requests_.push_back(std::move(request));
  }
}

void Camera::request_complete(libcamera::Request *request) {
  if (request->status() == libcamera::Request::RequestCancelled)
    return;

  processRequest(request);
}

void Camera::processRequest(libcamera::Request *request) {
  static int counter = 0;
  if (request->status() == libcamera::Request::RequestCancelled)
    return;

  queue_.add(new CompletedRequest(counter++, request));
}