#pragma once

#include <mutex>
#include <queue>
#include <condition_variable>



namespace rscamera {
	template <typename T>
	class Pipeline {


		public: 
			void add(const T & new_request);

			T pop();

		private: 
			std::queue<T> queue_;
			std::mutex mutex_;
			std::condition_variable cond_;
	};

	template <typename T>
	void Pipeline<T>::add(const T & new_request) {
		std::unique_lock<std::mutex> lock(mutex_);
		queue_.push(new_request);
		cond_.notify_one();
	}

	template <typename T>
	T Pipeline<T>::pop() {
		std::unique_lock<std::mutex> lock(mutex_);
		cond_.wait(lock, [this] { return !queue_.empty(); });
		T request  = queue_.front();
		queue_.pop();
		return request;
	}

}




