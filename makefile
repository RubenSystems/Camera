

all:
	g++ main.cpp src/camera.cpp  -I /usr/local/include/libcamera -I ./src/include -L /usr/lib/arm-linux-gnueabihf/ -l camera -l camera-base -std=c++17 -o main
