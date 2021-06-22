#include <hiptf/device.hpp>
#include <hiptf/rsmi.hpp>
#include "gpu_monitor_hip.hpp"

void mtk::gpu_monitor::gpu_monitor_hip::init() {
	HIPTF_CHECK_ERROR(rsmi_init(0));
}

void mtk::gpu_monitor::gpu_monitor_hip::shutdown() {
}

std::size_t mtk::gpu_monitor::gpu_monitor_hip::get_num_devices() const {
	return hiptf::device::get_num_devices();
}

std::size_t mtk::gpu_monitor::gpu_monitor_hip::get_current_device() const {
	return hiptf::device::get_device_id();
}

double mtk::gpu_monitor::gpu_monitor_hip::get_current_temperature(const unsigned gpu_id) const {
	int64_t temperature;
	HIPTF_CHECK_ERROR(rsmi_dev_temp_metric_get(gpu_id, 0, RSMI_TEMP_CURRENT, &temperature));

	return temperature / 1000.0;
}

double mtk::gpu_monitor::gpu_monitor_hip::get_current_power(const unsigned gpu_id) const {
	uint64_t power;
	HIPTF_CHECK_ERROR(rsmi_dev_power_cap_get(gpu_id, 0, &power));

	return power / 1000000.0;
}

std::size_t mtk::gpu_monitor::gpu_monitor_hip::get_current_used_memory(const unsigned gpu_id) const {
	uint64_t used;
	HIPTF_CHECK_ERROR(rsmi_dev_memory_usage_get(gpu_id, RSMI_MEM_TYPE_VRAM, &used));

	return used;
}
