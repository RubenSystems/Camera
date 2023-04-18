#pragma once

#include <mutex>
#include <queue>
#include <condition_variable>

#include <completed_request.h>




namespace rscamera {
	class RequestQueue {

		public:
			typedef CompletedRequest* request_type;

		public: 
			void add(const request_type & new_request) {
				std::unique_lock<std::mutex> lock(mutex_);
				queue_.push(new_request);
				cond_.notify_one();
			}

			request_type pop() {
				std::unique_lock<std::mutex> lock(mutex_);
				cond_.wait(lock, [this] { return !queue_.empty(); });
				request_type request  = queue_.front();
				queue_.pop();
				return request;
			}

		private: 
			std::queue<request_type> queue_;
			std::mutex mutex_;
			std::condition_variable cond_;
	};
}

