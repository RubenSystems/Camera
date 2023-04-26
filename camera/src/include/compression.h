#pragma once

#include <string>
#include <turbojpeg.h>
#include <opencv2/opencv.hpp>
#include <jpeglib.h>
#include <vector>
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
			Compresser(uint32_t width, uint32_t height, uint32_t stride);

			~Compresser ();
		
		public: 

			uint64_t compress(uint8_t * source_image);

			uint8_t * buffer();

			void free_buffer(uint8_t * compressed_buffer);

			void inc_quality ();

			void dec_quality();


		private:
			static constexpr uint8_t QUALITY_BUMP = 1;
			int quality = 30;
			static constexpr int COLOR_COMPONENTS = 3;
			uint8_t * buffer_;
			size_t buffer_size_;
			uint32_t width_, height_, stride_; 
			cv::Mat frame_; 
			std::vector<int> params_;
			// tjhandle compresser_;
	};
}
