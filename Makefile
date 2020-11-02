NVCC=nvcc
NVCCFLAGS=-std=c++11 -I./cutf -lnvidia-ml
TARGET=gpu_logger

$(TARGET):main.cu
	$(NVCC) $< $(NVCCFLAGS) -o $@

clean:
	rm -f $(TARGET)
