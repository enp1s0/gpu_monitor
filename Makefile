NVCC=nvcc
NVCCFLAGS=-std=c++11 -I./cutf -lnvidia-ml
TARGET=gpu_logger
SRCS=main.cu

ACC=CUDA

ifeq ($(ACC), CUDA)
SRCS+=gpu_logger_cuda.cu
endif

$(TARGET):$(SRCS)
	$(NVCC) $+ $(NVCCFLAGS) -o $@ -DACC_$(ACC)

clean:
	rm -f $(TARGET)
