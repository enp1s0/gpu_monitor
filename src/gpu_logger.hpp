#ifndef __GPU_LOGGER_HPP__
#define __GPU_LOGGER_HPP__
#include <cstdint>

namespace mtk {
class gpu_logger {
public:
	virtual void init() = 0;
	virtual void shutdown() = 0;

	virtual std::size_t get_num_devices() const = 0;

	virtual double get_current_temperature(const unsigned gpu_id) const = 0;
	virtual double get_current_power(const unsigned gpu_id) const = 0;
	virtual std::size_t get_current_used_memory(const unsigned gpu_id) const = 0;
};
} // namespace mtk
#endif
