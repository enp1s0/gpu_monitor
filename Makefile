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
HIPPATH=/opt/rocm/hip
CXX=$(HIPPATH)/bin/hipcc
HIP_INC=$(HIPPATH)/include
RSMI_INC=$(HIPPATH)/../rocm_smi/include
HIP_LIB=$(HIPPATH)/lib
RSMI_LIB=$(HIPPATH)/../rocm_smi/lib
CXXFLAGS+=-I./hiptf -I$(RSMI_INC) -I$(HIP_INC) -L$(RSMI_LIB) -L$(HIP_LIB) -lrocm_smi64 -lrt
SRCS+=gpu_logger_hip.cpp
endif

$(TARGET):$(SRCS)
	$(CXX) $+ $(CXXFLAGS) -o $@ -DACC_$(ACC)

clean:
	rm -f $(TARGET)
