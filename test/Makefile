CXX=g++
CXXFLAGS=-std=c++11 -I../include
NVCC=nvcc
NVCCFLAGS=$(CXXFLAGS) -arch=sm_80 -L../build -lgpu_monitor -I../src/cutf/include -lcublas -lnvidia-ml

TARGET=message_insert_test.cpp.out api.cu.out

all: $(TARGET)

%.cpp.out: %.cpp
	$(CXX) $< -o $@ $(CXXFLAGS)

%.cu.out: %.cu
	$(NVCC) $< -o $@ $(NVCCFLAGS)

clean:
	rm -f $(TARGET)
