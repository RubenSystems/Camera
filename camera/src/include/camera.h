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

#include <pipeline.h>
#include <completed_request.h>

/*
	=== Modes for the camera === 
	[{'bit_depth': 10,
	'crop_limits': (1048, 1042, 2560, 1440),
	'exposure_limits': (203, 85182424, None),
	'format': SRGGB10_CSI2P,
	'fps': 120.0,
	'size': (1280, 720),
	'unpacked': 'SRGGB10'},
	{'bit_depth': 10,
	'crop_limits': (408, 674, 3840, 2160),
	'exposure_limits': (282, 118430097, None),
	'format': SRGGB10_CSI2P,
	'fps': 60.05,
	'size': (1920, 1080),
	'unpacked': 'SRGGB10'},
	{'bit_depth': 10,
	'crop_limits': (0, 0, 4656, 3496),
	'exposure_limits': (305, 127960311, None),
	'format': SRGGB10_CSI2P,
	'fps': 30.0,
	'size': (2328, 1748),
	'unpacked': 'SRGGB10'},
	{'bit_depth': 10,
	'crop_limits': (408, 672, 3840, 2160),
	'exposure_limits': (491, 206049113, None),
	'format': SRGGB10_CSI2P,
	'fps': 18.0,
	'size': (3840, 2160),
	'unpacked': 'SRGGB10'},
	{'bit_depth': 10,
	'crop_limits': (0, 0, 4656, 3496),
	'exposure_limits': (592, 248567756, None),
	'format': SRGGB10_CSI2P,
	'fps': 9.0,
	'size': (4656, 3496),
	'unpacked': 'SRGGB10'
	}]

*/

#define CAMERA_WIDTH 2328
#define CAMERA_HEIGHT 1748
#define CAMERA_BUFFERS 2
#define CAMERA_FPS_STREAM 25
#define CAMERA_FPS_IDLE 25


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

			void set_idle();

			void set_streaming();


		private:

			void set_framerate(int fps);

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
			Pipeline<CompletedRequest *> queue_;

	};
}