#pragma once

#include <string>

#include <jpeglib.h>
#include <vector>
#include <pipeline.h>
#include <buffer_pool.h>


namespace rscamera {
	struct CompressedObject {
		uint8_t * object; 
		size_t size; 
	};

	class Compresser {

		public:
			Compresser(uint32_t width, uint32_t height, uint32_t stride, Pipeline<CompressedObject> *);

			~Compresser ();
		
		public: 

			void compress(uint8_t * source_image);

			void free_buffer(uint8_t * compressed_buffer);

			void inc_quality ();

			void dec_quality();

			CompressedObject dequeue();


		private:
			static constexpr uint8_t QUALITY_BUMP = 1;
			int quality = 30;
			uint8_t current_buffer_ = 0; 
			static constexpr int COLOR_COMPONENTS = 3;
			Pipeline<CompressedObject> * pipe_;
			uint32_t width_, height_, stride_; 
			BufferPool buffers_; 
			std::mutex mutex_;
			
	};
}
