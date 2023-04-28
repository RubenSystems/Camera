#include <buffer_pool.h>

using namespace rscamera; 

BufferPool::BufferPool(uint8_t count, size_t size): 
	buffer_count_(count), buffer_size_(size) {

		buffers_ = new uint8_t * [count]; 
		for (int i = 0; i < count; i ++) {
			buffers_[i] = new uint8_t [size];
		}

}

BufferPool::~BufferPool() {
	for (int i = 0; i < buffer_count_; i ++) {
		delete[] buffers_[i];
	}
	delete[] buffers_;
}

uint8_t * BufferPool::buffer(uint8_t idx) {
	return buffers_[idx];
}

size_t BufferPool::buffer_size() {
	return buffer_size_;
}