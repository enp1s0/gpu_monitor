#ifndef __GPU_LOGGER_CUDA_HPP__
#define __GPU_LOGGER_CUDA_HPP__
#include "gpu_logger.hpp"

namespace mtk {
class gpu_logger_hip : public mtk::gpu_logger {
public:
	void init();
	void shutdown();

	std::size_t get_num_devices() const;

	double get_current_temperature(const unsigned gpu_id) const;
	double get_current_power(const unsigned gpu_id) const;
	std::size_t get_current_used_memory(const unsigned gpu_id) const;
};
} // namespace mtk
#endif
