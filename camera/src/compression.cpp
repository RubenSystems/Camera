#include <chrono>
#include <compression.h>
#include <iostream>
using namespace rscamera;

Compresser::Compresser(uint32_t width, uint32_t height, uint32_t stride,
                       Pipeline<CompressedObject> *egress_pipe)
    : width_(width), height_(height), stride_(stride), pipe_(egress_pipe),
      buffers_(10, width * height) {

  cinfo_.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo_);
}

Compresser::~Compresser() {
  // tjDestroy(compresser_);
  // delete [] buffer_;
  jpeg_destroy_compress(&cinfo_);
}

// From libcameara apps:
void Compresser::yuv420_to_jpeg(const uint8_t *input, uint8_t **output,
                                size_t *jpeg_len) {
  cinfo_.image_width = width_;
  cinfo_.image_height = height_;
  cinfo_.input_components = 3;
  cinfo_.in_color_space = JCS_YCbCr;
  cinfo_.restart_interval = 0;

  jpeg_set_defaults(&cinfo_);
  cinfo_.raw_data_in = TRUE;
  jpeg_set_quality(&cinfo_, quality, TRUE);
  jpeg_mem_dest(&cinfo_, output, jpeg_len);
  jpeg_start_compress(&cinfo_, TRUE);

  int stride2 = stride_ / 2;
  uint8_t *Y = (uint8_t *)input;
  uint8_t *U = (uint8_t *)Y + stride_ * height_;
  uint8_t *V = (uint8_t *)U + stride2 * (height_ / 2);
  uint8_t *Y_max = U - stride_;
  uint8_t *U_max = V - stride2;
  uint8_t *V_max = U_max + stride2 * (height_ / 2);

  JSAMPROW y_rows[16];
  JSAMPROW u_rows[8];
  JSAMPROW v_rows[8];

  for (uint8_t *Y_row = Y, *U_row = U, *V_row = V;
       cinfo_.next_scanline < height_;) {
    for (int i = 0; i < 16; i++, Y_row += stride_)
      y_rows[i] = std::min(Y_row, Y_max);
    for (int i = 0; i < 8; i++, U_row += stride2, V_row += stride2)
      u_rows[i] = std::min(U_row, U_max), v_rows[i] = std::min(V_row, V_max);

    JSAMPARRAY rows[] = {y_rows, u_rows, v_rows};
    jpeg_write_raw_data(&cinfo_, rows, 16);
  }

  jpeg_finish_compress(&cinfo_);
}

void Compresser::compress(uint8_t *source_image) {

  size_t current_buffer_index;
  {
    std::unique_lock<std::mutex> lock(mutex_);
    current_buffer_index = current_buffer_;
    current_buffer_ = (current_buffer_ + 1) % 10;
  }
  uint8_t *buffer = buffers_.buffer(current_buffer_index);
  size_t size = buffers_.buffer_size();

  yuv420_to_jpeg(source_image, &buffer, &size);

  pipe_->add({buffer, size});
}

rscamera::CompressedObject Compresser::dequeue() { return pipe_->pop(); }

void Compresser::inc_quality() { quality += QUALITY_BUMP; }

void Compresser::dec_quality() { quality -= QUALITY_BUMP; }
