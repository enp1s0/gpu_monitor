#include <cutf/device.hpp>
#include <cutf/nvml.hpp>
#include "gpu_logger_cuda.hpp"

void mtk::gpu_logger_cuda::init() {
	CUTF_CHECK_ERROR(nvmlInit());
}

void mtk::gpu_logger_cuda::shutdown() {
	CUTF_CHECK_ERROR(nvmlShutdown());
}

std::size_t mtk::gpu_logger_cuda::get_num_devices() const {
	return cutf::device::get_num_devices();
}

double mtk::gpu_logger_cuda::get_current_temperature(const unsigned gpu_id) const {
	nvmlDevice_t device;
	CUTF_CHECK_ERROR(nvmlDeviceGetHandleByIndex(gpu_id, &device));
	unsigned int temperature;
	CUTF_CHECK_ERROR(nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &temperature));

	return temperature;
}

double mtk::gpu_logger_cuda::get_current_power(const unsigned gpu_id) const {
	nvmlDevice_t device;
	CUTF_CHECK_ERROR(nvmlDeviceGetHandleByIndex(gpu_id, &device));
	unsigned int power;
	CUTF_CHECK_ERROR(nvmlDeviceGetPowerUsage(device, &power));

	return power / 1000.0;
}

std::size_t mtk::gpu_logger_cuda::get_current_used_memory(const unsigned gpu_id) const {
	nvmlDevice_t device;
	CUTF_CHECK_ERROR(nvmlDeviceGetHandleByIndex(gpu_id, &device));
	nvmlMemory_t memory;
	CUTF_CHECK_ERROR(nvmlDeviceGetMemoryInfo(device, &memory));

	return memory.used;
}
