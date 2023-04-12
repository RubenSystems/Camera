#pragma once

#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>

#include <libcamera/camera.h>
#include <libcamera/camera_manager.h>
#include <libcamera/control_ids.h>
#include <libcamera/controls.h>
#include <libcamera/formats.h>
#include <libcamera/framebuffer_allocator.h>
#include <libcamera/property_ids.h>

#define CAMERA_HEIGHT 640
#define CAMERA_WIDTH 1024

namespace rscamera {
	class Camera {

		public:
			Camera() ;

			~Camera();

		public: 
			void start();

		private:

			static void request_complete(libcamera::Request *request);

			void init_camera();

			void configure_camera();

			void create_buffer_allocator();

			void configure_requests();

			void processRequest(libcamera::Request *request) {
				std::cout << std::endl
					<< "Request completed: " << request->toString() << std::endl;

				/*
				* When a request has completed, it is populated with a metadata control
				* list that allows an application to determine various properties of
				* the completed request. This can include the timestamp of the Sensor
				* capture, or its gain and exposure values, or properties from the IPA
				* such as the state of the 3A algorithms.
				*
				* ControlValue types have a toString, so to examine each request, print
				* all the metadata for inspection. A custom application can parse each
				* of these items and process them according to its needs.
				*/
				const libcamera::ControlList &requestMetadata = request->metadata();
				for (const auto &ctrl : requestMetadata) {
					const libcamera::ControlId *id = libcamera::controls::controls.at(ctrl.first);
					const libcamera::ControlValue &value = ctrl.second;

					std::cout << "\t" << id->name() << " = " << value.toString()
						<< std::endl;
				}

				/*
				* Each buffer has its own FrameMetadata to describe its state, or the
				* usage of each buffer. While in our simple capture we only provide one
				* buffer per request, a request can have a buffer for each stream that
				* is established when configuring the camera.
				*
				* This allows a viewfinder and a still image to be processed at the
				* same time, or to allow obtaining the RAW capture buffer from the
				* sensor along with the image as processed by the ISP.
				*/
				const libcamera::Request::BufferMap &buffers = request->buffers();
				for (auto bufferPair : buffers) {
					// (Unused) Stream *stream = bufferPair.first;
					libcamera::FrameBuffer *buffer = bufferPair.second;
					const libcamera::FrameMetadata &metadata = buffer->metadata();

					/* Print some information about the buffer which has completed. */
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

					/*
					* Image data can be accessed here, but the FrameBuffer
					* must be mapped by the application
					*/
				}

				/* Re-queue the Request to the camera. */
				request->reuse(libcamera::Request::ReuseBuffers);
				camera_->queueRequest(request);
			}



		private:
			libcamera::CameraManager * manager_;
			std::shared_ptr<libcamera::Camera> camera_ = nullptr;
			libcamera::ControlList controls_;
			std::unique_ptr<libcamera::CameraConfiguration> config_;
			libcamera::StreamConfiguration * stream_config_;
			libcamera::FrameBufferAllocator *allocator_;
			std::vector<std::unique_ptr<libcamera::Request>> requests_;

	};
}