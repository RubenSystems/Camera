
#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>

#include <string.h>

#include <camera.h>
#include <rsics_server.h>

#include <opencv2/opencv.hpp>


int main() {

	rscamera::Camera camera;
	std::thread th1([&camera](){
		camera.start();	
	});

	rsics::BroadcastServer server;

	server.start();

	cv::Mat frame; 
	frame.create(camera.dimensions().height, camera.dimensions().width, CV_8UC3);

	int counter = 0;

	std::vector<uint8_t> encoder_buffer;
	std::vector<int> jpeg_write_params; 
	jpeg_write_params.resize(9, 0);
	jpeg_write_params[0] = cv::IMWRITE_JPEG_QUALITY;
	jpeg_write_params[1] = 20;
	jpeg_write_params[2] = cv::IMWRITE_JPEG_PROGRESSIVE;
	jpeg_write_params[3] = 0;
	jpeg_write_params[4] = cv::IMWRITE_JPEG_OPTIMIZE;
	jpeg_write_params[5] = 0;
	jpeg_write_params[6] = cv::IMWRITE_JPEG_RST_INTERVAL;
	jpeg_write_params[7] = 0;

	while (true) {
		// this waits untill there is a complete request
		rscamera::CompletedRequest * req = camera.completed_request();
		libcamera::Stream * camera_stream = camera.stream();
		
		libcamera::FrameBuffer * buffer = req->buffers[camera_stream];
		std::vector<libcamera::Span<uint8_t>> frame_buffer = camera.buffer(buffer);

		uint8_t * frame_memory = frame_buffer[0].data();
		for (uint32_t i = 0; i < camera.dimensions().height; i++, frame_memory += camera.dimensions().stride) {
			memmove(frame.ptr(i), frame_memory, camera.dimensions().width * 3);
		}
		cv::imencode(".jpg", frame, encoder_buffer );

		server.broadcast(encoder_buffer.data(), encoder_buffer.size());
		std::cout << encoder_buffer.size() << " " <<  encoder_buffer.size() / 1465 << std::endl;			


		camera.next_frame(req);
		delete req;
	}
	std::cout << "hello" << std::endl;
	return 0; 
}