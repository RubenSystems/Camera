
#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>

#include <libcamera/camera.h>
#include <libcamera/camera_manager.h>
#include <libcamera/control_ids.h>
#include <libcamera/controls.h>
#include <libcamera/formats.h>
#include <libcamera/framebuffer_allocator.h>
#include <libcamera/property_ids.h>

#include "src/include/camera.h"



int main() {

	rscamera::Camera camera;
	camera.start();

	uint16_t x = 5000;
	std::this_thread::sleep_for(std::chrono::milliseconds(x));

	

	std::cout << "hello" << std::endl;
	return 0; 
}