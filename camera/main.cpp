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
			const std::vector<int> & core_mapping ): threads_(core_mapping.size()) {


			for (int i = 0; i < (int)core_mapping.size(); i ++) {
				threads_[i] = std::thread([ingress_pipeline, egress_pipeline, stride](){
					
					rscamera::Compresser compresser_(CAMERA_WIDTH, CAMERA_HEIGHT, stride, egress_pipeline);
					while (true) {
						std::vector<libcamera::Span<uint8_t>> frame = ingress_pipeline->pop();
						uint8_t * frame_memory = frame[0].data();
						compresser_.compress(frame_memory);
					}
				});
				set_thread_affinity(threads_[i], core_mapping[i]);
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

	server.set_on_client_change([&camera](int no_clients){
		if (no_clients == 0) {
			camera.set_idle();
		} else {
			camera.set_streaming();
		}
	});

	uint64_t frames_processed = 0; 
	auto start = std::chrono::high_resolution_clock::now();


	server.start();

	rscamera::Pipeline<std::vector<libcamera::Span<uint8_t>>> frame_pipeline;
	rscamera::Pipeline<rscamera::CompressedObject> compression_pipeline; 
	CompressionPool compression_pool(&frame_pipeline, &compression_pipeline, camera.dimensions().stride, {0, 1, 2, 3});

	
	std::thread sending_thread; 
	sending_thread = std::thread([
		&server,
		&compression_pipeline,
		&frames_processed,
		&sending_thread
	](){
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
		// std::cout << frames_per_second << "\n";
		// std::cout << frame_pipeline.count() << std::endl;		

		camera.next_frame(req);
		delete req;
	}
	std::cout << "hello" << std::endl;
	return 0; 
}