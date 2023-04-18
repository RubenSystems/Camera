CXX = g++
CPPFLAGS = -std=c++17 -DDEBUG
INCLUDES = -I /usr/include/libcamera -I ./src/include -I /usr/local/include/opencv4 
LIBS = `pkg-config --libs --cflags libcamera` `pkg-config --libs --cflags opencv4` -pthread
SRCS = $(shell find ./src -name "*.cpp")  main.cpp
OBJS = $(patsubst %.cpp, %.o, $(SRCS))
TARGET = main


all: main

main: $(OBJS)
	$(CXX) $(CPPFLAGS) -o $(TARGET) $(OBJS) $(INCLUDES) $(LIBS)

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(INCLUDES) -c $< -o $@


clean: 
	rm  $(TARGET) $(OBJS)