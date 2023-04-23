#pragma once

#include <string>
#include <turbojpeg.h>

/*

	const int JPEG_QUALITY = 75;
	const int COLOR_COMPONENTS = 3;
	int _width = 1920;
	int _height = 1080;
	long unsigned int _jpegSize = 0;
	unsigned char* _compressedImage = NULL; //!< Memory is allocated by tjCompress2 if _jpegSize == 0
	unsigned char buffer[_width*_height*COLOR_COMPONENTS]; //!< Contains the uncompressed image

	tjhandle _jpegCompressor = tjInitCompress();

	tjCompress2(_jpegCompressor, buffer, _width, 0, _height, TJPF_RGB,
			&_compressedImage, &_jpegSize, TJSAMP_444, JPEG_QUALITY,
			TJFLAG_FASTDCT);

	tjDestroy(_jpegCompressor);

	//to free the memory allocated by TurboJPEG (either by tjAlloc(), 
	//or by the Compress/Decompress) after you are done working on it:
	tjFree(&_compressedImage);

*/


namespace rscamera {
	class Compresser {

		public:
			Compresser(uint32_t width, uint32_t height):
				width_(width), height_(height) {
				compresser_ = tj3Init(TJINIT_COMPRESS);
				std::cout << tj3Set(compresser_, TJPARAM_QUALITY, 5) << "\n";
				std::cout << tj3Set(compresser_, TJPARAM_SUBSAMP, TJSAMP_444) << "\n";
				std::cout << tj3Set(compresser_, TJPARAM_FASTDCT, 1) << "\n";
				buffer_size_ = tj3JPEGBufSize(width, height, TJSAMP_444 );
				buffer_ = tjAlloc(buffer_size_);
			}

			~Compresser () {
				tjDestroy(compresser_);
			}
		
		public: 

			uint64_t compress(uint8_t * source_image) {
				uint64_t size = buffer_size_;

				tj3Compress8(
					compresser_, 
					source_image, 
					width_, 
					0, 
					height_, 
					TJPF_RGB,
					&buffer_, 
					&size
					// ,
					// TJSAMP_444, 
					// quality,
					// TJFLAG_FASTDCT | TJFLAG_NOREALLOC
				);
				std::cout << (void *)buffer_ << std::endl;
				return size;
			}

			uint8_t * buffer() {
				return buffer_;
			}

			void free_buffer(uint8_t * compressed_buffer) {
				tjFree(compressed_buffer);
			}

			void inc_quality () {
				// quality += QUALITY_BUMP;
			}

			void dec_quality() {
				// quality -= QUALITY_BUMP;
			}


		private:
			static constexpr uint8_t QUALITY_BUMP = 1;
			int quality = 30;
			static constexpr int COLOR_COMPONENTS = 3;
			uint8_t * buffer_;
			size_t buffer_size_;
			uint32_t width_, height_; 
			tjhandle compresser_;
	};
}
