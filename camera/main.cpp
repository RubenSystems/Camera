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
#include <jpeg_converter.h>




#define FRAME_COUNT_RANGE 5
#define FRAME_OPTIMISATION_JUMP 2
#define FRAME_COUNT 40
#define BYTES_PER_FRAME 1465








int main() {

	rscamera::Camera camera;
	std::thread camera_thread([&camera](){
		camera.start();	
	});

	rscamera::Compresser compresser (CAMERA_WIDTH, CAMERA_HEIGHT); 


	rsics::BroadcastServer server;
	uint64_t frames_processed = 0; 

	server.start();

	// std::vector<uint8_t> encoder_buffer;
	// std::vector<int> jpeg_write_params; 
	// jpeg_write_params.resize(8, 0);
	// jpeg_write_params[0] = cv::IMWRITE_JPEG_QUALITY;
	// jpeg_write_params[1] = 70;
	// jpeg_write_params[2] = cv::IMWRITE_JPEG_PROGRESSIVE;
	// jpeg_write_params[3] = 0;
	// jpeg_write_params[4] = cv::IMWRITE_JPEG_OPTIMIZE;
	// jpeg_write_params[5] = 0;
	// jpeg_write_params[6] = cv::IMWRITE_JPEG_RST_INTERVAL;
	// jpeg_write_params[7] = 0;
	// int & jpeg_optimisation = jpeg_write_params[1];


	// cv::Mat frame; 
	// frame.create(camera.dimensions().height, camera.dimensions().width, CV_8UC3);

	std::queue<std::vector<libcamera::Span<uint8_t>>> send_frames_queue;
	std::condition_variable cond;
	std::mutex mut; 
	std::mutex comp_mut; 


	std::thread send_thread([
		&send_frames_queue, 
		&cond, 
		&mut, 
		&comp_mut, 
		&camera, 
		&server,
		&frames_processed,
		&compresser


	](){
		while (true) {
			
			std::vector<libcamera::Span<uint8_t>> frame_buffer;
			{
				std::unique_lock<std::mutex> lock(mut);
				cond.wait(lock, [&send_frames_queue] { return !send_frames_queue.empty(); });
				frame_buffer = send_frames_queue.front();
				send_frames_queue.pop();
			}

			uint8_t * frame_memory = frame_buffer[0].data();

			uint64_t size;
			{
				std::unique_lock<std::mutex> lock(comp_mut);
				size = compresser.compress(frame_memory);
				server.broadcast(compresser.buffer(), size);			
			}
			std::cout << size / BYTES_PER_FRAME << std::endl;
			// compresser.free_buffer(jpeg_buffer);
			// if (encoder_buffer.size() / BYTES_PER_FRAME > FRAME_COUNT + FRAME_COUNT_RANGE) {
			// 	compresser.dec_quality();
			// } else if (encoder_buffer.size() / BYTES_PER_FRAME < FRAME_COUNT - FRAME_COUNT_RANGE) {
			// 	compresser.inc_quality();
			// }
			frames_processed++;

		}
	});
	


	auto start = std::chrono::high_resolution_clock::now();

	while (true) {
		// this waits untill there is a complete request
		
		rscamera::CompletedRequest * req = camera.completed_request();


		libcamera::Stream * camera_stream = camera.stream();
		
		libcamera::FrameBuffer * buffer = req->buffers[camera_stream];
		std::vector<libcamera::Span<uint8_t>> frame_buffer = camera.buffer(buffer);

		std::unique_lock<std::mutex> lock(mut);
		send_frames_queue.push(frame_buffer);
		cond.notify_one();
		

		// uint8_t * frame_memory = frame_buffer[0].data();
		// for (uint32_t i = 0; i < camera.dimensions().height; i++, frame_memory += camera.dimensions().stride) {
		// 	memmove(frame.ptr(i), frame_memory, camera.dimensions().width * 3);
		// }
		
		
		// cv::imencode(".jpg", frame, encoder_buffer,  jpeg_write_params);

		// server.broadcast(encoder_buffer.data(), encoder_buffer.size());
		// // std::cout << encoder_buffer.size() << " " <<  encoder_buffer.size() / 1465 << std::endl;			
		// if (encoder_buffer.size() / BYTES_PER_FRAME > FRAME_COUNT + FRAME_COUNT_RANGE) {
		// 	jpeg_optimisation -= FRAME_OPTIMISATION_JUMP; 
		// } else if (encoder_buffer.size() / BYTES_PER_FRAME < FRAME_COUNT - FRAME_COUNT_RANGE) {
		// 	jpeg_optimisation += FRAME_OPTIMISATION_JUMP; 
		// }
		

		auto current = std::chrono::high_resolution_clock::now();
		auto difference = std::chrono::duration_cast<std::chrono::seconds>(current - start).count();
		uint64_t frames_per_second = frames_processed / difference; 
		// std::cout << frames_per_second << "\n";

		camera.next_frame(req);
		delete req;
	}
	std::cout << "hello" << std::endl;
	return 0; 
}