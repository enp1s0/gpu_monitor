NVCC=nvcc
NVCCFLAGS=-std=c++11
TARGET=gpu_logger

$(TARGET):main.cu
	$(NVCC) $< $(NVCCFLAGS) -o $@

clean:
	rm -f $(TARGET)
