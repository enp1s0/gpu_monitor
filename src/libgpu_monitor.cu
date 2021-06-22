#include <iostream>
#include <sstream>
#include <random>
#include <string>
#include <ctime>
#include <chrono>
#include <thread>
#include <gpu_monitor/gpu_monitor.h>
#include <unistd.h>

#ifdef ACC_CUDA
#include "gpu_monitor_cuda.hpp"
#endif

#ifdef ACC_HIP
#include "gpu_monitor_hip.hpp"
#endif

std::vector<std::tuple<std::time_t, double, double, double>> mtk::gpu_monitor::measure_power_consumption(
		const std::function<void(void)> func,
		const std::time_t interval
		) {
	std::vector<std::tuple<std::time_t, double, double, double>> profiling_result;

#ifdef ACC_CUDA
	mtk::gpu_monitor::gpu_monitor_cuda gpu_monitor;
#endif
#ifdef ACC_HIP
	mtk::gpu_monitor::gpu_monitor_hip gpu_monitor;
#endif
	gpu_monitor.init();
	int gpu_id = gpu_monitor.get_current_device();
	// Output log
	unsigned count = 0;

	// Start target thread
	std::thread thread(func);

	// Start measurement
	const auto start_clock = std::chrono::high_resolution_clock::now();
	do {
		const auto end_clock = std::chrono::high_resolution_clock::now();
		const auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end_clock - start_clock).count();

		// call measurement functions
		const auto temperature = gpu_monitor.get_current_temperature(gpu_id);
		const auto power = gpu_monitor.get_current_power(gpu_id);
		const auto memory_consumption = gpu_monitor.get_current_used_memory(gpu_id);

		const auto end_clock_1 = std::chrono::high_resolution_clock::now();
		const auto elapsed_time_1 = std::chrono::duration_cast<std::chrono::microseconds>(end_clock_1 - start_clock).count();

		// Store data
		profiling_result.push_back(std::make_tuple(temperature, power, memory_consumption, static_cast<std::time_t>(elapsed_time_1)));

		// Sleep
		usleep(std::max<std::time_t>(interval * 1000 * count, elapsed_time_1) - elapsed_time_1);
	} while (!thread.joinable());

	thread.join();

	gpu_monitor.shutdown();

	return profiling_result;
}
