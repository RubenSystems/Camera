#pragma once 

#define CAMERA_WIDTH 1920
#define CAMERA_HEIGHT 1080
#define CAMERA_BUFFERS 2
#define CAMERA_FPS_STREAM 30
#define CAMERA_FPS_IDLE 30

#define DEBUG_PRINT 0
#define QUEUE_MAX_SIZE 2

#define COMPRESSION_QUALITY 60


#if DEBUG_PRINT
	#include <iostream>
#endif