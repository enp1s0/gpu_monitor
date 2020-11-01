NVCC=nvcc
NVCCFLAGS=-std=c++11 -I./cutf
TARGET=gpu_logger

$(TARGET):main.cu
	$(NVCC) $< $(NVCCFLAGS) -o $@

clean:
	rm -f $(TARGET)
