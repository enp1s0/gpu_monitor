#include <cutf/device.hpp>
#include <cutf/nvml.hpp>
#include <cutf/device.hpp>
#include "gpu_monitor_cuda.hpp"

void mtk::gpu_monitor::gpu_monitor_cuda::init() {
	CUTF_CHECK_ERROR(nvmlInit());
}

void mtk::gpu_monitor::gpu_monitor_cuda::shutdown() {
	CUTF_CHECK_ERROR(nvmlShutdown());
}

std::size_t mtk::gpu_monitor::gpu_monitor_cuda::get_num_devices() const {
	return cutf::device::get_num_devices();
}

std::size_t mtk::gpu_monitor::gpu_monitor_cuda::get_current_device() const {
	return cutf::device::get_device_id();
}

double mtk::gpu_monitor::gpu_monitor_cuda::get_current_temperature(const unsigned gpu_id) const {
	nvmlDevice_t device;
	CUTF_CHECK_ERROR_M(nvmlDeviceGetHandleByIndex(gpu_id, &device), "GPU ID = " + std::to_string(gpu_id));
	unsigned int temperature;
	CUTF_CHECK_ERROR_M(nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &temperature), "GPU ID = " + std::to_string(gpu_id));

	return temperature;
}

double mtk::gpu_monitor::gpu_monitor_cuda::get_current_power(const unsigned gpu_id) const {
	nvmlDevice_t device;
	CUTF_CHECK_ERROR_M(nvmlDeviceGetHandleByIndex(gpu_id, &device), "GPU ID = " + std::to_string(gpu_id));
	unsigned int power;
	CUTF_CHECK_ERROR_M(nvmlDeviceGetPowerUsage(device, &power), "GPU ID = " + std::to_string(gpu_id));

	return power / 1000.0;
}

std::size_t mtk::gpu_monitor::gpu_monitor_cuda::get_current_used_memory(const unsigned gpu_id) const {
	nvmlDevice_t device;
	CUTF_CHECK_ERROR_M(nvmlDeviceGetHandleByIndex(gpu_id, &device), "GPU ID = " + std::to_string(gpu_id));
	nvmlMemory_t memory;
	CUTF_CHECK_ERROR_M(nvmlDeviceGetMemoryInfo(device, &memory), "GPU ID = " + std::to_string(gpu_id));

	return memory.used;
}

std::vector<std::pair<unsigned, std::string>> mtk::gpu_monitor::gpu_monitor_cuda::get_gpu_list() const {
	const auto props = cutf::device::get_properties_vector();

	std::vector<std::pair<unsigned, std::string>> res;
	unsigned device_id = 0;
	for (const auto& p : props) {
		res.push_back(std::make_pair<unsigned, std::string>(device_id++, p.name));
	}

	return res;
}
