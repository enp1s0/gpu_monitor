CXX=
CXXFLAGS=-std=c++11
TARGET=gpu_logger
SRCS=main.cpp

ACC=CUDA

ifeq ($(ACC), CUDA)
CXX=nvcc
CXXFLAGS+=-I./cutf -lnvidia-ml
SRCS+=gpu_logger_cuda.cu
endif
ifeq ($(ACC), HIP)
SRCS+=gpu_logger_hip.cpp
endif

$(TARGET):$(SRCS)
	$(CXX) $+ $(CXXFLAGS) -o $@ -DACC_$(ACC)

clean:
	rm -f $(TARGET)
