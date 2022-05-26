#ifndef __GPU_LOGGER_CUDA_HPP__
#define __GPU_LOGGER_CUDA_HPP__
#include "gpu_monitor.hpp"

namespace mtk {
namespace gpu_monitor {
class gpu_monitor_cuda : public mtk::gpu_monitor::gpu_monitor {
public:
	void init();
	void shutdown();

	std::size_t get_num_devices() const;
	std::size_t get_current_device() const;

	double get_current_temperature(const unsigned gpu_id) const;
	double get_current_power(const unsigned gpu_id) const;
	std::size_t get_current_used_memory(const unsigned gpu_id) const;

	std::vector<std::pair<unsigned, std::string>> get_gpu_list() const;
};
} // namespace gpu_monitor
} // namespace mtk
#endif
