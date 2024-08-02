#include <iostream>
#include <sstream>
#include <random>
#include <string>
#include <ctime>
#include <chrono>
#include <thread>
#include <gpu_monitor/gpu_monitor.hpp>

#ifdef ACC_CUDA
#include "gpu_monitor_cuda.hpp"
#endif

#ifdef ACC_HIP
#include "gpu_monitor_hip.hpp"
#endif

std::vector<mtk::gpu_monitor::profiling_data> mtk::gpu_monitor::measure_power_consumption(
		const std::function<void(void)> func,
		const std::time_t interval
		) {
	std::vector<mtk::gpu_monitor::profiling_data> profiling_result;

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

	int semaphore = 1;

	// Start target thread
	std::thread thread(
			[&](){func();semaphore = 0;}
			);

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
		const auto elapsed_time_1 = std::chrono::duration_cast<std::chrono::milliseconds>(end_clock_1 - start_clock).count();

		// Store data
		profiling_result.push_back(mtk::gpu_monitor::profiling_data{temperature, power, memory_consumption, static_cast<std::time_t>(elapsed_time)});

		// Sleep
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(std::chrono::milliseconds(std::max<std::time_t>(static_cast<int>(interval) * count, elapsed_time_1) - elapsed_time_1));
		count++;
	} while (semaphore);

	thread.join();

	gpu_monitor.shutdown();

	return profiling_result;
}

double mtk::gpu_monitor::get_integrated_power_consumption(
		const std::vector<mtk::gpu_monitor::profiling_data>& profiling_data_list
		) {
	if (profiling_data_list.size() == 0) {
		return 0.0;
	}

	double power_consumption = 0.;
	for (unsigned i = 1; i < profiling_data_list.size(); i++) {
		const auto elapsed_time = (profiling_data_list[i].timestamp - profiling_data_list[i - 1].timestamp) * 1e-6;
		// trapezoidal integration
		power_consumption += (profiling_data_list[i].power + profiling_data_list[i - 1].power) / 2 * elapsed_time;
	}
	return power_consumption;
}

double mtk::gpu_monitor::get_elapsed_time(
		const std::vector<mtk::gpu_monitor::profiling_data>& profiling_data_list
		) {
	if (profiling_data_list.size() == 0) {
		return 0.0;
	}
	return (profiling_data_list[profiling_data_list.size() - 1].timestamp - profiling_data_list[0].timestamp) * 1e-6;
}
