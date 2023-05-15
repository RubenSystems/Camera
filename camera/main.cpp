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

#include <sched.h>
#include <pthread.h>

#include <buffer_pool.h>


#define FRAME_COUNT_RANGE 2
#define FRAME_COUNT 100
#define BYTES_PER_FRAME 1465


void set_thread_affinity(std::thread & thread, int core) {
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(core, &cpuset);
	int rc = pthread_setaffinity_np(
		thread.native_handle(), 
		sizeof(cpu_set_t), 
		&cpuset
	);
	std::cout << rc << std::endl;
	if (rc < 0) {

		perror("THREAD_AFFINITY: ");
		exit(0);
	}
}

class CompressionPool {
	public :
		CompressionPool(
			rscamera::Pipeline<std::vector<libcamera::Span<uint8_t>>> * ingress_pipeline, 
			rscamera::Pipeline<rscamera::CompressedObject> * egress_pipeline, 
			int stride, 
			const std::vector<int> & core_mapping ) {
			for (int i = 0; i < core_mapping.size(); i ++) {
				std::thread x; 
				x = std::thread([ingress_pipeline, egress_pipeline, &core_mapping, &x, i, stride](){
					// std::cout << core_mapping[i] << std::endl;
					// set_thread_affinity(x, core_mapping[i]);
					rscamera::Compresser compresser (CAMERA_WIDTH, CAMERA_HEIGHT, stride, egress_pipeline); 
					while (true) {
						std::vector<libcamera::Span<uint8_t>> frame = ingress_pipeline->pop();
						uint8_t * frame_memory = frame[0].data();
						compresser.compress(frame_memory);
					}
				});
				threads_.push_back(std::move(x));
			}
		}
	private: 
		std::vector<std::thread> threads_;  
		
};


int main() {

	rscamera::Camera camera;
	std::thread camera_thread; 
	camera_thread = std::thread([&camera, &camera_thread](){
		set_thread_affinity(camera_thread, 0);
		camera.start();	
	});

	


	rsics::BroadcastServer server;
	uint64_t frames_processed = 0; 
	auto start = std::chrono::high_resolution_clock::now();


	server.start();

	rscamera::Pipeline<std::vector<libcamera::Span<uint8_t>>> frame_pipeline;
	rscamera::Pipeline<rscamera::CompressedObject> compression_pipeline; 
	CompressionPool compression_pool(&frame_pipeline, &compression_pipeline, camera.dimensions().stride, {1,2,3});
	// std::thread compression_thread; 
	// compression_thread = std::thread([
	// 	&frame_pipeline,
	// 	&compresser,
	// 	&compression_thread
	// ](){
	// 	set_thread_affinity(compression_thread, 1);
	// 	while (true) {
			
	// 		std::vector<libcamera::Span<uint8_t>> frame = frame_pipeline.pop();
	// 		uint8_t * frame_memory = frame[0].data();
	// 		compresser.compress(frame_memory);
	// 	}
	// });

	// std::thread compression_thread2; 
	// compression_thread2 = std::thread([
	// 	&frame_pipeline,
	// 	&compresser,
	// 	&compression_thread2
	// ](){
	// 	set_thread_affinity(compression_thread2, 3);
	// 	while (true) {
			
	// 		std::vector<libcamera::Span<uint8_t>> frame = frame_pipeline.pop();
	// 		uint8_t * frame_memory = frame[0].data();
	// 		compresser.compress(frame_memory);
	// 	}
	// });

	// std::thread compression_thread3; 
	// compression_thread3 = std::thread([
	// 	&frame_pipeline,
	// 	&compresser,
	// 	&compression_thread3
	// ](){
	// 	set_thread_affinity(compression_thread3, 2);
	// 	while (true) {
			
	// 		std::vector<libcamera::Span<uint8_t>> frame = frame_pipeline.pop();
	// 		uint8_t * frame_memory = frame[0].data();
	// 		compresser.compress(frame_memory);
	// 	}
	// });
	
	
	std::thread sending_thread; 
	sending_thread = std::thread([
		&server,
		&compression_pipeline,
		&frames_processed,
		&sending_thread
	](){
		set_thread_affinity(sending_thread, 2);
		while (true) {
			rscamera::CompressedObject frame = compression_pipeline.pop();
			server.broadcast(frame.object, frame.size);
			// if (frame.size / BYTES_PER_FRAME > FRAME_COUNT + FRAME_COUNT_RANGE) {
			// 	compresser.dec_quality();
			// } else if (frame.size / BYTES_PER_FRAME < FRAME_COUNT - FRAME_COUNT_RANGE) {
			// 	compresser.inc_quality();
			// }
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
		std::cout << frames_per_second << "\n";
		// std::cout << frame_pipeline.count() << std::endl;		

		camera.next_frame(req);
		delete req;
	}
	std::cout << "hello" << std::endl;
	return 0; 
}