#include <compression.h>

using namespace rscamera; 

Compresser::Compresser(uint32_t width, uint32_t height, uint32_t stride):
	width_(width), height_(height), stride_(stride) {

	frame_.create(height, width, CV_8UC3);

	// params_.resize(8, 0);
	// params_[0] = cv::IMWRITE_JPEG_QUALITY;
	// params_[1] = 20;
	// params_[2] = cv::IMWRITE_JPEG_PROGRESSIVE;
	// params_[3] = 0;
	// params_[4] = cv::IMWRITE_JPEG_OPTIMIZE;
	// params_[5] = 0;
	// params_[6] = cv::IMWRITE_JPEG_RST_INTERVAL;
	// params_[7] = 0;

	buffer_ = new uint8_t [width * height * 8];

	// compresser_ = tj3Init(TJINIT_COMPRESS);
	// tj3Set(compresser_, TJPARAM_QUALITY, 5) ;
	// tj3Set(compresser_, TJPARAM_SUBSAMP, TJSAMP_444);
	// tj3Set(compresser_, TJPARAM_FASTDCT, 1);
	// tj3Set(compresser_, TJPARAM_NOREALLOC, 1);

	// // tj3SetScalingFactor(compresser_, 0.5); 

	// buffer_size_ = tj3JPEGBufSize(width, height, TJSAMP_444 );
	// buffer_ = tjAlloc(buffer_size_);
}

Compresser::~Compresser () {
	// tjDestroy(compresser_);
	// delete [] buffer_;
}



static void YUV420_to_JPEG_fast(const uint8_t *input, uint32_t width, uint32_t height, uint32_t stride,
								const int quality, const unsigned int restart,
								uint8_t **jpeg_buffer, size_t &jpeg_len)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_YCbCr;
	cinfo.restart_interval = restart;

	jpeg_set_defaults(&cinfo);
	cinfo.raw_data_in = TRUE;
	jpeg_set_quality(&cinfo, quality, TRUE);
	*jpeg_buffer = NULL;
	jpeg_len = 0;
	jpeg_mem_dest(&cinfo, jpeg_buffer, &jpeg_len);
	jpeg_start_compress(&cinfo, TRUE);

	int stride2 = stride / 2;
	uint8_t *Y = (uint8_t *)input;
	uint8_t *U = (uint8_t *)Y + stride * height;
	uint8_t *V = (uint8_t *)U + stride2 * (height / 2);
	uint8_t *Y_max = U - stride;
	uint8_t *U_max = V - stride2;
	uint8_t *V_max = U_max + stride2 * (height / 2);

	JSAMPROW y_rows[16];
	JSAMPROW u_rows[8];
	JSAMPROW v_rows[8];

	for (uint8_t *Y_row = Y, *U_row = U, *V_row = V; cinfo.next_scanline < height;)
	{
		for (int i = 0; i < 16; i++, Y_row += stride)
			y_rows[i] = std::min(Y_row, Y_max);
		for (int i = 0; i < 8; i++, U_row += stride2, V_row += stride2)
			u_rows[i] = std::min(U_row, U_max), v_rows[i] = std::min(V_row, V_max);

		JSAMPARRAY rows[] = { y_rows, u_rows, v_rows };
		jpeg_write_raw_data(&cinfo, rows, 16);
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
}

uint64_t Compresser::compress(uint8_t * source_image) {

	size_t size = width_ * height_ * 8; 
	YUV420_to_JPEG_fast(
		source_image,
		width_, 
		height_, 
		stride_,
		10,
		0,
		&buffer_,
		size
	);
	return size;

	// for (uint32_t i = 0; i < height_; i++, source_image += stride_) {
	// 	memmove(frame_.ptr(i), source_image, width_ * 3);
	// }

	// cv::imencode(".jpg", frame_, buffer_, params_);
	// return buffer_.size();
	// uint64_t size = buffer_size_;

	// tj3Compress8(
	// 	compresser_, 
	// 	source_image, 
	// 	width_, 
	// 	0, 
	// 	height_, 
	// 	TJ_YUV,
	// 	&buffer_, 
	// 	&size
	// 	// ,
	// 	// TJSAMP_444, 
	// 	// quality,
	// 	// TJFLAG_FASTDCT | TJFLAG_NOREALLOC
	// );
	// return size;
}

uint8_t * Compresser::buffer() {
	return buffer_;
}

void Compresser::free_buffer(uint8_t * compressed_buffer) {
	// tjFree(compressed_buffer);
}

void Compresser::inc_quality () {
	// quality += QUALITY_BUMP;
}

void Compresser::dec_quality() {
	// quality -= QUALITY_BUMP;
}