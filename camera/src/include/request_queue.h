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
			void add(const request_type & new_request);

			request_type pop();

		private: 
			std::queue<request_type> queue_;
			std::mutex mutex_;
			std::condition_variable cond_;
	};
}

