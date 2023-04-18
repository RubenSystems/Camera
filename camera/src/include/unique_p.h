#pragma once

#include <functional>

namespace rscamera {
	template <typename T>
	class unique_p {

		public:
			typedef std::function<void (T *)> custom_deleter;

			unique_p (T * raw, custom_deleter deleter) : data_(raw), deleter_(deleter) {}

			~unique_p() {
				if (deleter_)
					deleter_(data_);

				delete data_;
			}
			
			unique_p(unique_p && rhs) {
				operator = (rhs);
			}

			unique_p * operator = (unique_p && rhs) {
				data_ = rhs.data;
				deleter_ = rhs.deleter_; 

				rhs.data_ = nullptr; 
				rhs.deleter_ = nullptr;
			}

			// Not copyable
			unique_p(const unique_p & rhs) = delete;
			unique_p * operator = (const unique_p & rhs) = delete;

		
		public:
			T * data() {
				return data_;
			}

		private:
			T * data_;
			custom_deleter deleter_;
	};
}