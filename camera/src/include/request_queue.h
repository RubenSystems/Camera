#pragma once

#include <mutex>
#include <queue>
#include <condition_variable>

#include <unique_p.h>
#include <completed_request.h>


template <typename T>
struct QueueNode {
	QueueNode(const T & data_): data(data_), next(nullptr) {}

	T data; 
	QueueNode<T> * next; 
};

template <typename T>
class Queue {

	public: 

		void push(const T & data) {
			QueueNode<T> node = new QueueNode<T>(data);
			if (empty()) {
				front_ = node; 
				back_ = node; 
			} else {
				back_->next = node;
			}
			size_++; 
		}

		T pop() {
			if (empty())
				throw std::runtime_error("[QUEUE] - popping from empty queue");

			QueueNode<T> res = *front_; 
			T data = std::move(res.data);
			
			front_ = front_->next; 
			delete res; 

			if (front_ == nullptr)
				back_ = nullptr;

			return data;
		}

		bool empty() {
			return size_ == 0; 
		}

		size_t size() {
			return size_; 
		}

	private: 
		QueueNode<T> * front_ = nullptr;
		QueueNode<T> * back_ = nullptr; 
		size_t size_ = 0;
		
};



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

