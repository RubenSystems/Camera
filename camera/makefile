# /usr/local/lib/arm-linux-gnueabihf/pkgconfig
# TARGETS = src/camera.cpp
# INCLUDES = -I /usr/local/include/libcamera -I ./src/include -I /usr/local/include/opencv4 
# LIBRARIES = -L /usr/lib/arm-linux-gnueabihf/ -l camera -l camera-base `pkg-config --libs opencv4` `pkg-config --cflags opencv4`
# CPPVER = -std=c++17

# TARGET = main

# all:
# 	g++ main.cpp $(TARGETS) $(INCLUDES) $(LIBRARIES) $(CPPVER) -o $(TARGET)

# clean: 
# 	rm $(TARGET)

CXX = g++
CPPFLAGS = -std=c++17 -DDEBUG
INCLUDES = -I /usr/local/include/libcamera -I ./src/include -I /usr/local/include/opencv4 
LIBS = -L /usr/lib/arm-linux-gnueabihf/ -l camera -l camera-base `pkg-config --libs --cflags opencv4`
SRCS = $(shell find ./src -name "*.cpp")
OBJS = $(patsubst %.cpp, %.o, $(SRCS))
MAIN = main.cpp
TARGET = main


all: main

main: $(OBJS)
	$(CXX) $(CPPFLAGS) $(MAIN) -o $(TARGET) $(OBJS) $(INCLUDES) $(LIBS)

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(INCLUDES) -c $< -o $@


clean: 
	rm  $(TARGET) $(OBJS)