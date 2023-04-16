
#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>

#include <camera.h>



int main() {

	rscamera::Camera camera;
	camera.start();

	uint16_t x = 5000;
	std::this_thread::sleep_for(std::chrono::milliseconds(x));

	

	std::cout << "hello" << std::endl;
	return 0; 
}