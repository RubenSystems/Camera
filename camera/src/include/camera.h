#pragma once

#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>
#include <unordered_map>

#include <libcamera/camera.h>
#include <libcamera/camera_manager.h>
#include <libcamera/control_ids.h>
#include <libcamera/controls.h>
#include <libcamera/formats.h>
#include <libcamera/framebuffer_allocator.h>
#include <libcamera/property_ids.h>

#include <request_queue.h>

#define CAMERA_HEIGHT 3496 / 2
#define CAMERA_WIDTH 4656 / 2
#define DEBUG_PRINT 0

namespace rscamera {


	struct Dimensions {
		uint32_t width, height, stride;
	};

	class Camera {

		public:
			Camera();

			~Camera();

		public: 
			void start();

			Dimensions dimensions();

			libcamera::Stream * stream();

			CompletedRequest * completed_request();

			bool has_buffer(libcamera::FrameBuffer * pointer);

			std::vector<libcamera::Span<uint8_t>> buffer(libcamera::FrameBuffer * pointer);

			void next_frame(CompletedRequest * req);

		private:

			// Initalisation/configuration
			void init_camera();

			void configure_camera();

			void create_buffer_allocator();

			void configure_requests();
			
			// Event handlers
			void request_complete(libcamera::Request *request);

			void processRequest(libcamera::Request *request);

			void get_dimensions();
		
		private:
			libcamera::CameraManager * manager_;
			std::shared_ptr<libcamera::Camera> camera_ = nullptr;
			libcamera::ControlList controls_;
			std::unique_ptr<libcamera::CameraConfiguration> config_;
			libcamera::StreamConfiguration * stream_config_;
			libcamera::FrameBufferAllocator * allocator_;
			std::vector<std::unique_ptr<libcamera::Request>> requests_;
			libcamera::Stream * stream_;
			std::unordered_map<
				libcamera::FrameBuffer *, 
				std::vector<libcamera::Span<uint8_t>>
			> mapped_buffers_;
			Dimensions dimensions_;
			RequestQueue queue_;

	};
}