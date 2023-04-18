
#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>

#include <string.h>

#include <camera.h>


#include <opencv2/opencv.hpp>



int main() {

	rscamera::Camera camera;
	std::thread th1([&camera](){
		camera.start();	
	});

	cv::Mat frame; 
	frame.create(camera.dimensions().height, camera.dimensions().width, CV_8UC3);

	int counter = 0;
	while (counter ++ < 40) {
		// this waits untill there is a complete request
		rscamera::CompletedRequest * req = camera.completed_request();
		libcamera::Stream * camera_stream = camera.stream();
		
		libcamera::FrameBuffer * buffer = req->buffers[camera_stream];
		std::vector<libcamera::Span<uint8_t>> frame_buffer = camera.buffer(buffer);

		uint8_t * frame_memory = frame_buffer[0].data();
		for (uint32_t i = 0; i < camera.dimensions().height; i++, frame_memory += camera.dimensions().stride) {
			memmove(frame.ptr(i), frame_memory, camera.dimensions().width * 3);
		}
		cv::imwrite( "testing/" + std::to_string(counter) + ".jpg", frame);

		camera.next_frame(req);
		delete req;
	}

	std::cout << "hello" << std::endl;
	return 0; 
}