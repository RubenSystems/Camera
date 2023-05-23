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

			Compresser(const Compresser & ) = delete; 
			Compresser & operator = (const Compresser &) = delete;
			
			Compresser (Compresser &&) = delete; 
			Compresser & operator =(Compresser &&) = delete;

			~Compresser ();
		
		public: 

			void compress(uint8_t * source_image);

			void inc_quality ();

			void dec_quality();

			CompressedObject dequeue();

		private:
			void yuv420_to_jpeg( const uint8_t *input,
								uint8_t **jpeg_buffer, size_t * jpeg_len);


		private:
			static constexpr uint8_t QUALITY_BUMP = 1;
			static constexpr int COLOR_COMPONENTS = 3;

			struct jpeg_compress_struct cinfo_;
			struct jpeg_error_mgr jerr;
			

			int quality = 70;
			uint8_t current_buffer_ = 0; 
			
			uint32_t width_, height_, stride_; 
			Pipeline<CompressedObject> * pipe_;
			
			BufferPool buffers_; 
			std::mutex mutex_;
			
			
	};
}
