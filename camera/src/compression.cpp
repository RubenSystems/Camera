#include <compression.h>
#include <iostream>
using namespace rscamera; 

Compresser::Compresser(uint32_t width, uint32_t height, uint32_t stride, Pipeline<CompressedObject> * egress_pipe):
	width_(width), height_(height), stride_(stride), buffers_(10, width * height), pipe_(egress_pipe) {
}

Compresser::~Compresser () {
	// tjDestroy(compresser_);
	// delete [] buffer_;
}



static void YUV420_to_JPEG_fast(const uint8_t *input, uint32_t width, uint32_t height, uint32_t stride,
								const int quality, const unsigned int restart,
								uint8_t **jpeg_buffer, size_t * jpeg_len)
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
	// *jpeg_len = 0;
	jpeg_set_quality(&cinfo, quality, TRUE);
	jpeg_mem_dest(&cinfo, jpeg_buffer, jpeg_len);
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

void Compresser::compress(uint8_t * source_image) {
	size_t current_buffer_index; 
	{
		std::unique_lock<std::mutex> lock(mutex_);
		current_buffer_index = current_buffer_;
		current_buffer_ = (current_buffer_ + 1) % 10;
	}
	uint8_t * buffer = buffers_.buffer(current_buffer_index);
	size_t size = buffers_.buffer_size();


	YUV420_to_JPEG_fast(
		source_image,
		width_, 
		height_, 
		stride_,
		quality,
		0,
		&buffer,
		&size
	);
	
	pipe_->add({
		buffer,
		size
	});

}

rscamera::CompressedObject Compresser::dequeue() {
	return pipe_->pop();
}

void Compresser::inc_quality () {
	quality += QUALITY_BUMP;
}

void Compresser::dec_quality() {
	quality -= QUALITY_BUMP;
}
