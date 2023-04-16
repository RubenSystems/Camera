
#include <camera.h>
#include <sys/mman.h>

// temporary
#include <opencv2/opencv.hpp>

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

/*
	Privats
*/
void Camera::init_camera() {
	int ret = manager_->start();
	for (auto & i : manager_->cameras()) {
		std::cout << "NEW CAM\n";
		camera_ = i;
		break;
	}
}

// [5:34:31.988634869] [95190]  INFO Camera camera_manager.cpp:299 libcamera v0.0.0+4225-74d023d8
// [5:34:32.056062840] [95193]  INFO RPI raspberrypi.cpp:1476 Registered camera /base/soc/i2c0mux/i2c@1/imx519@1a to Unicam device /dev/media4 and ISP device /dev/media1
// [5:34:32.057752434] [95190]  INFO Camera camera.cpp:1028 configuring streams: (0) 2328x1748-YUV420
// [5:34:32.058968515] [95193]  INFO RPI raspberrypi.cpp:851 Sensor: /base/soc/i2c0mux/i2c@1/imx519@1a - Selected sensor format: 2328x1748-SRGGB10_1X10 - Selected unicam format: 2328x1748-pRAA

void Camera::configure_camera() {

	config_ = camera_->generateConfiguration(
		{ libcamera::StreamRole::Viewfinder }
	);

	stream_config_ = &(config_->at(0));

	stream_config_->size.width = CAMERA_WIDTH;
	stream_config_->size.height = CAMERA_HEIGHT; 
	stream_config_->bufferCount = 4; 
	stream_config_->pixelFormat = libcamera::formats::RGB888;


	config_->validate();
	camera_->configure(config_.get());

	controls_.set(libcamera::controls::ExposureTime, 0.0);
	controls_.set(libcamera::controls::AeExposureMode, libcamera::controls::ExposureNormal);
	controls_.set(libcamera::controls::AwbMode, libcamera::controls::AwbAuto);
	controls_.set(libcamera::controls::AeMeteringMode, libcamera::controls::MeteringMatrix);
	controls_.set(libcamera::controls::AfMode, libcamera::controls::AfModeEnum::AfModeAuto);
}

void Camera::get_dimensions() {
	libcamera::StreamConfiguration const &cfg = stream_->configuration();
	Dimensions ret = {
		stream_config_->size.width, 
		stream_config_->size.height,
		stream_config_->stride
	};
	dimensions_ = ret;

}


void Camera::create_buffer_allocator() {
	allocator_ = new libcamera::FrameBufferAllocator(camera_);


	for (libcamera::StreamConfiguration &cfg : *config_) {
		if (allocator_->allocate(cfg.stream()) < 0) {
			throw std::runtime_error("[CAMERA] - can't allocate buffers");
		}
		for (const std::unique_ptr<libcamera::FrameBuffer> & buffer: allocator_->buffers(cfg.stream())) {
			size_t buffer_size = 0;
			for (unsigned i = 0; i < buffer->planes().size(); i++) {
				const libcamera::FrameBuffer::Plane &plane = buffer->planes()[i];
				buffer_size += plane.length;
				if (i == buffer->planes().size() - 1 || plane.fd.get() != buffer->planes()[i + 1].fd.get()) {
					void * memory = mmap(NULL, buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, plane.fd.get(), 0);
					mapped_buffers_[buffer.get()].push_back(
						libcamera::Span<uint8_t>(static_cast<uint8_t *>(memory), buffer_size)
					);
					buffer_size = 0;
				}
			}

		}
	}
}

void Camera::configure_requests() {
	stream_ = stream_config_->stream();
	const std::vector<std::unique_ptr<libcamera::FrameBuffer>> & buffers = allocator_->
																			buffers(stream_);
	
	for (unsigned int i = 0; i < buffers.size(); ++i) {
		std::unique_ptr<libcamera::Request> request = camera_->createRequest();
		if (!request) {
			throw std::runtime_error("[CAMERA] - cannot make request");
		}

		libcamera::FrameBuffer * buffer = buffers[i].get();
		int ret = request->addBuffer(stream_,  buffer);
		if (ret < 0) {
			throw std::runtime_error("[CAMERA] - cannot set buffer");
		}

		libcamera::ControlList &controls = request->controls();
		// controls.set(libcamera::controls::Brightness, 0.5);

		requests_.push_back(std::move(request));
	}
}


void Camera::request_complete(libcamera::Request *request) {
	processRequest(request);
}


void Camera::processRequest(libcamera::Request *request) {
	std::cout << std::endl
		<< "Request completed: " << request->toString() << std::endl;
	int counter = 0;
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
		

		#if 1
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
		#endif

		// IMAGE HERE!
		auto item = mapped_buffers_.find(buffer);
		
		
		if (item != mapped_buffers_.end()) {
			std::vector<libcamera::Span<uint8_t>> img = item->second;
			
			cv::Mat frame; 
			frame.create(dimensions_.height, dimensions_.width, CV_8UC3);
			
			uint8_t * memory = item->second[0].data();
			for (uint32_t i = 0; i < dimensions_.height; i++, memory += dimensions_.stride) {
				memmove(frame.ptr(i), memory, dimensions_.width * 3);
				// std::cout << (int)*memory << " " << dimensions_.stride << std::endl;
			}

			cv::imwrite( "testing/" + std::to_string(counter++) + ".jpg", frame);

		} 
	}

	/* Re-queue the Request to the camera. */
	request->reuse(libcamera::Request::ReuseBuffers);
	// camera_->queueRequest(request);
}