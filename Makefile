NVCC=nvcc
NVCCFLAGS=-std=c++11 -I./cutf -lnvidia-ml
TARGET=gpu_logger
SRCS=main.cpp

ACC=CUDA

ifeq ($(ACC), CUDA)
SRCS+=gpu_logger_cuda.cu
endif
ifeq ($(ACC), HIP)
SRCS+=gpu_logger_hip.cpp
endif

$(TARGET):$(SRCS)
	$(NVCC) $+ $(NVCCFLAGS) -o $@ -DACC_$(ACC)

clean:
	rm -f $(TARGET)
