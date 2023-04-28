#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include <string.h>

#include <camera.h>
#include <rsics_server.h>
#include <compression.h>

#include <buffer_pool.h>


#define FRAME_COUNT_RANGE 2
#define FRAME_OPTIMISATION_JUMP 2
#define FRAME_COUNT 80
#define BYTES_PER_FRAME 1465


int main() {

	rscamera::Camera camera;
	std::thread camera_thread([&camera](){
		camera.start();	
	});

	rscamera::Compresser compresser (CAMERA_WIDTH, CAMERA_HEIGHT, camera.dimensions().stride); 


	rsics::BroadcastServer server;
	uint64_t frames_processed = 0; 
	auto start = std::chrono::high_resolution_clock::now();


	server.start();

	rscamera::Pipeline<std::vector<libcamera::Span<uint8_t>>> frame_pipeline;

	std::thread compression_thread([
		&frame_pipeline,
		&compresser
	](){
		while (true) {
			
			std::vector<libcamera::Span<uint8_t>> frame = frame_pipeline.pop();
			uint8_t * frame_memory = frame[0].data();
			compresser.compress(frame_memory);
		}
	});
	

	std::thread sending_thread([
		&server,
		&compresser,
		&frames_processed
	](){
		while (true) {
			rscamera::CompressedObject frame = compresser.dequeue();
			server.broadcast(frame.object, frame.size);
			if (frame.size / BYTES_PER_FRAME > FRAME_COUNT + FRAME_COUNT_RANGE) {
				compresser.dec_quality();
			} else if (frame.size / BYTES_PER_FRAME < FRAME_COUNT - FRAME_COUNT_RANGE) {
				compresser.inc_quality();
			}

			frames_processed++;
		}
	});
	
	while (true) {
		
		rscamera::CompletedRequest * req = camera.completed_request();


		libcamera::Stream * camera_stream = camera.stream();
		
		libcamera::FrameBuffer * buffer = req->buffers[camera_stream];
		std::vector<libcamera::Span<uint8_t>> frame_buffer = camera.buffer(buffer);

		frame_pipeline.add(frame_buffer);

		

		auto current = std::chrono::high_resolution_clock::now();
		auto difference = std::chrono::duration_cast<std::chrono::seconds>(current - start).count();
		uint64_t frames_per_second = frames_processed / difference; 
		// std::cout << frames_per_second << "\n";
		// std::cout << frame_pipeline.count() << std::endl;		

		camera.next_frame(req);
		delete req;
	}
	std::cout << "hello" << std::endl;
	return 0; 
}