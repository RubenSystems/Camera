CXX = g++
CPPFLAGS = -std=c++17 -O3 -ftree-vectorize -Wall -Wextra -g
INCLUDES = -I /usr/include/libcamera -I ./src/include -I ./lib/RSiCS/src/include -I /opt/libjpeg-turbo/include/ -I /usr/local/include/opencv4
LIBS = `pkg-config --libs --cflags libcamera` `pkg-config --libs --cflags libturbojpeg` `pkg-config --libs --cflags libjpeg` `pkg-config --libs --cflags opencv4` -pthread -L/lib/RSiCS/ -lrsics2 
# `pkg-config --libs --cflags opencv4`
# -L/opt/libjpeg-turbo/lib64/ -ljpeg -lturbojpeg
SRCS = $(shell find ./src -name "*.cpp")  main.cpp
HDRS = SRCS = $(shell find ./src/include -name "*.h")  main.cpp
OBJS = $(patsubst %.cpp, %.o, $(SRCS))
TARGET = main


all: main

main: $(OBJS)
	$(CXX) $(CPPFLAGS) -o $(TARGET) $(OBJS) $(INCLUDES) $(LIBS)

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

format:
	clang-format -i -style="file:clang-format.txt" src/*.cpp src/include/*.h

clean: 
	rm  $(TARGET) $(OBJS)