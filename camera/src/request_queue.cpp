#include <request_queue.h>

using namespace rscamera;


void RequestQueue::add(const request_type & new_request) {
	std::unique_lock<std::mutex> lock(mutex_);
	queue_.push(new_request);
	cond_.notify_one();
}

RequestQueue::request_type RequestQueue::pop() {
	std::unique_lock<std::mutex> lock(mutex_);
	cond_.wait(lock, [this] { return !queue_.empty(); });
	request_type request  = queue_.front();
	queue_.pop();
	return request;
}


