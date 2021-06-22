CXX=
LIBDIR=lib
SRCDIR=src
OBJDIR=obj
CXXFLAGS=-std=c++17 -I./include -lstdc++fs -L./$(LIBDIR)
TARGET_BIN=gpu_monitor
TARGET_LIB=$(LIBDIR)/libgpu_monitor.a
OBJS=

ACC=CUDA

CXXFLAGS+=-DACC_$(ACC)

ifeq ($(ACC), CUDA)
CXX=nvcc
CXXFLAGS+=-I./cutf -lnvidia-ml
OBJS+=$(OBJDIR)/gpu_monitor_cuda.o
endif
#ifeq ($(ACC), HIP)
#HIPPATH=/opt/rocm/hip
#CXX=$(HIPPATH)/bin/hipcc
#HIP_INC=$(HIPPATH)/include
#RSMI_INC=$(HIPPATH)/../rocm_smi/include
#HIP_LIB=$(HIPPATH)/lib
#RSMI_LIB=$(HIPPATH)/../rocm_smi/lib
#CXXFLAGS+=-I./hiptf -I$(RSMI_INC) -I$(HIP_INC) -L$(RSMI_LIB) -L$(HIP_LIB) -lrocm_smi64 -lrt
#OBJS+=$(OBJDIR)+=src/gpu_monitor_hip.o
#endif

all: $(TARGET_BIN) $(TARGET_LIB)

$(TARGET_BIN):$(SRCDIR)/main.o $(OBJS)
	$(CXX) $+ $(CXXFLAGS) -o $@

$(TARGET_LIB):$(SRCDIR)/libgpu_monitor.o $(OBJS)
	$(CXX) $+ $(CXXFLAGS) -lib -o $@

$(SRCDIR)/%.cpp: $(SRCDIR)/%.cu
	$(CXX) $< $(CXXFLAGS) --cuda -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	[ -d $(OBJDIR) ] || mkdir $(OBJDIR)
	$(CXX) $< $(CXXFLAGS) -c -o $@

clean:
	rm -f $(TARGET)
