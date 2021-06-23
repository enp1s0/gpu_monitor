#include <iostream>
#include <gpu_monitor/gpu_monitor.hpp>
#include <cutf/memory.hpp>
#include <cutf/cublas.hpp>

constexpr std::size_t N = 1lu << 15;

int main() {
	auto mat = cutf::memory::get_device_unique_ptr<float>(N * N);

	auto cublas_handle = cutf::cublas::get_cublas_unique_ptr();

	auto lauch_gemm = [&]() {
		const float a = 1.0f;
		for (unsigned i = 0; i < 10; i++) {
			CUTF_CHECK_ERROR(cutf::cublas::gemm(
						*cublas_handle.get(),
						CUBLAS_OP_N, CUBLAS_OP_N,
						N, N, N,
						&a,
						mat.get(), N,
						mat.get(), N,
						&a,
						mat.get(), N
						));
		}
		cudaDeviceSynchronize();
	};

	// Profiling
	const auto progiling_result = mtk::gpu_monitor::measure_power_consumption(
			lauch_gemm,
			20
			);

	// Get max temperature and power
	double max_temperature = 0.;
	double max_power = 0.;
	for (const auto& pr : progiling_result) {
		max_temperature = std::max(max_temperature, pr.temperature);
		max_power       = std::max(max_power      , pr.power      );
	}

	double elapsed_time = (progiling_result[progiling_result.size() - 1].timestamp - progiling_result[0].timestamp) * 1e-6;

	std::printf("Num data        : %lu\n", progiling_result.size());
	std::printf("Max power       : %e [W]\n", max_power);
	std::printf("Max temperature : %e [C]\n", max_temperature);
	std::printf("Elapsed tiem    : %e [s]\n", elapsed_time);
}
