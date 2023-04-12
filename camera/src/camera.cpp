
#include "include/camera.h"

using rscamera::Camera; 

/*
	Initalisers
*/

Camera::Camera() {
	manager_ = new libcamera::CameraManager();
	init_camera();
	if (camera_ == nullptr) {
		throw std::runtime_error("[CAMERA] - no camera");
	}
	camera_->acquire();
	configure_camera();
	create_buffer_allocator();
	configure_requests();

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
	camera_->start();
	for (std::unique_ptr<libcamera::Request> &request : requests_)
		camera_->queueRequest(request.get());
}

/*
	Privats
*/

void Camera::request_complete(libcamera::Request *request) {
	processRequest(request);
}


void Camera::init_camera() {
	int ret = manager_->start();
	for (auto & i : manager_->cameras()) {
		camera_ = i;
		break;
	}
}


void Camera::configure_camera() {

	config_ = camera_
	->generateConfiguration(
		{ libcamera::StreamRole::VideoRecording }
	);

	stream_config_ = &(config_->at(0));

	stream_config_->size.width = CAMERA_WIDTH;
	stream_config_->size.height = CAMERA_HEIGHT; 

	config_->validate();
	camera_->configure(config_.get());
}

void Camera::create_buffer_allocator() {
	allocator_ = new libcamera::FrameBufferAllocator(camera_);


	for (libcamera::StreamConfiguration &cfg : *config_) {
		int ret = allocator_->allocate(cfg.stream());
		if (ret < 0) {
			throw std::runtime_error("[CAMERA] - can't allocate buffers");
		}

		size_t allocated = allocator_->buffers(cfg.stream()).size();
	}
}


void Camera::configure_requests() {
	libcamera::Stream *stream = stream_config_->stream();
	const std::vector<std::unique_ptr<libcamera::FrameBuffer>> &buffers = allocator_->
																			buffers(stream);
	
	for (unsigned int i = 0; i < buffers.size(); ++i) {
		std::unique_ptr<libcamera::Request> request = camera_->createRequest();
		if (!request) {
			throw std::runtime_error("[CAMERA] - cannot make request");
		}

		libcamera::FrameBuffer * buffer = buffers[i].get();
		int ret = request->addBuffer(stream,  buffer);
		if (ret < 0) {
			throw std::runtime_error("[CAMERA] - cannot set buffer");
		}

		libcamera::ControlList &controls = request->controls();
		// controls.set(50, libcamera::controls::Brightness);

		requests_.push_back(std::move(request));
	}
}

void Camera::processRequest(libcamera::Request *request) {
	std::cout << std::endl
		<< "Request completed: " << request->toString() << std::endl;

	const libcamera::ControlList &requestMetadata = request->metadata();
	for (const auto &ctrl : requestMetadata) {
		const libcamera::ControlId *id = libcamera::controls::controls.at(ctrl.first);
		const libcamera::ControlValue &value = ctrl.second;

		std::cout << "\t" << id->name() << " = " << value.toString()
			<< std::endl;
	}

	const libcamera::Request::BufferMap &buffers = request->buffers();
	for (auto bufferPair : buffers) {
		libcamera::FrameBuffer *buffer = bufferPair.second;
		const libcamera::FrameMetadata &metadata = buffer->metadata();

		std::cout << " seq: " << std::setw(6) << std::setfill('0') << metadata.sequence
			<< " timestamp: " << metadata.timestamp
			<< " bytesused: ";

		unsigned int nplane = 0;
		for (const libcamera::FrameMetadata::Plane &plane : metadata.planes())
		{
			std::cout << plane.bytesused;
			if (++nplane < metadata.planes().size())
				std::cout << "/";
		}

		std::cout << std::endl;

		// IMAGE HERE!
	}

	/* Re-queue the Request to the camera. */
	request->reuse(libcamera::Request::ReuseBuffers);
}