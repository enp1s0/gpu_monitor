#include <iostream>
#include <sstream>
#include <fstream>
#include <random>
#include <string>
#include <ctime>
#include <chrono>
#include <vector>
#include <exception>
#include <filesystem>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <gpu_monitor/gpu_monitor.hpp>

#ifdef ACC_CUDA
#include "gpu_monitor_cuda.hpp"
#endif

#ifdef ACC_HIP
#include "gpu_monitor_hip.hpp"
#endif

#include "cpu_vmem.hpp"

// e.g. Input `str` is "0,1,3", then return vector will be `{0, 1, 3}`.
std::vector<unsigned> get_gpu_ids(const std::string str) {
	std::stringstream ss(str);
	std::string buffer;
	std::vector<unsigned> result;
	while (std::getline(ss, buffer, ',')) {
		result.push_back(std::stoul(buffer));
	}
	return result;
}

int parse_params(unsigned &time_interval, std::string& output_file_name, std::vector<unsigned>& gpu_ids, int& run_command_head, int& set_default_gpus, int& print_result, int argc, char** argv) {
	run_command_head = 1;
	output_file_name = "gpu.csv";
	time_interval = 100;
	set_default_gpus = 0;
	gpu_ids = std::vector<unsigned>{0};
	print_result = 0;
	for (int i = 1; i < argc;) {
		if (std::string(argv[i]) == "-i") {
			if (i + 1 >= argc) {
				throw std::runtime_error("The value of `-i` was not provided");
			}
			time_interval = std::stoul(argv[i+1]);
			i += 2;
		} else if (std::string(argv[i]) == "-o") {
			if (i + 1 >= argc) {
				throw std::runtime_error("The value of `-o` was not provided");
			}
			output_file_name = argv[i+1];
			i += 2;
		} else if (std::string(argv[i]) == "-g") {
			if (i + 1 >= argc) {
				throw std::runtime_error("The value of `-g` was not provided");
			}
			if (std::string(argv[i+1]) != "ALL") {
				gpu_ids = get_gpu_ids(argv[i+1]);
				set_default_gpus = 1;
			}
			i += 2;
		} else if (std::string(argv[i]) == "-r") {
			print_result = 1;
			i += 1;
		} else if (std::string(argv[i]) == "-h") {
			return 1;
		} else if (std::string(argv[i]).substr(0, 1) == "-") {
			const std::string error_message = "Not supported option : " + std::string(argv[i]);
			std::fprintf(stderr, "[GPU logger ERROR] %s\n", error_message.c_str());
			return 1;
		} else {
			run_command_head = i;
			break;
		}
	}
	return 0;
}

void print_help_message(const char* const program_name) {
	std::printf("/*** GPU Logger ***/\n");
	std::printf("\n");
	std::printf("// Usage\n");
	std::printf("%s [-i interval(ms){default=100}] [-o output_file_name{default=gpu.csv}] [-g gpu_ids{default=ALL}] [-r] target_command\n", program_name);
}

namespace process {
constexpr char running = 'R';
constexpr char end     = 'E';
} // namespace process

namespace {
void insert_message(const std::string filename, std::ofstream& ofs) {
	std::ifstream ifs(filename);
	if (!ifs) {
		return;
	}

	std::string buffer;
	while (std::getline(ifs, buffer)) {
		if (buffer == "\n" || buffer.length() == 0) {
			continue;
		}
		ofs << buffer << std::endl;
	}
	ifs.close();

	std::ofstream ofs_m(filename);
	ofs_m.close();
}
} // noname namespace

int main(int argc, char** argv) {
	std::string output_file_name;
	unsigned time_interval;
	std::vector<unsigned> gpu_ids;
	int run_command_head;
	int set_default_gpus;
	int print_result;

	const auto res = parse_params(time_interval, output_file_name, gpu_ids, run_command_head, set_default_gpus, print_result, argc, argv);

	if (res > 0 || time_interval < 1 || argc <= 1) {
		print_help_message(argv[0]);
#ifdef ACC_CUDA
		mtk::gpu_monitor::gpu_monitor_cuda gpu_monitor;
#endif
#ifdef ACC_HIP
		mtk::gpu_monitor::gpu_monitor_hip gpu_monitor;
#endif
		gpu_monitor.init();
		const auto gpu_list = gpu_monitor.get_gpu_list();
		std::printf("\n");
		std::printf("// GPU List\n");
		for (const auto& p : gpu_list) {
			std::printf("%2u : %s\n", p.first, p.second.c_str());
		}
		gpu_monitor.shutdown();
		return 1;
	}

	const auto fd = shm_open("/gpu_monitor_smem", O_CREAT | O_RDWR, 0666);
	ftruncate(fd, 1);
	const auto semaphore = static_cast<char*>(mmap(nullptr, 1, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
	*semaphore = process::running;

	const auto fd_main_pid = shm_open("/cpu_monitor_smem", O_CREAT | O_RDWR, 0666);
	ftruncate(fd_main_pid, sizeof(std::uint32_t));
	const auto target_pid_ptr = static_cast<std::uint32_t*>(mmap(nullptr, 1, PROT_READ | PROT_WRITE, MAP_SHARED, fd_main_pid, 0));
	*target_pid_ptr = 0;

	// interprocess message
	std::string temp_dir = std::filesystem::temp_directory_path();
	const auto rand = std::random_device{}();
	std::string message_file_path = temp_dir + "/gm-" + std::to_string(rand);
	setenv(mtk::gpu_monitor::message_file_path_env_name, message_file_path.c_str(), 1);

	const auto pid = fork();
	if (pid == 0) {
#ifdef ACC_CUDA
		mtk::gpu_monitor::gpu_monitor_cuda gpu_monitor;
#endif
#ifdef ACC_HIP
		mtk::gpu_monitor::gpu_monitor_hip gpu_monitor;
#endif
		gpu_monitor.init();
		std::ofstream ofs(output_file_name);
		const auto num_devices = gpu_monitor.get_num_devices();

		if (set_default_gpus == 0) {
			gpu_ids = std::vector<unsigned>(num_devices);
			for (unsigned i = 0; i < num_devices; i++) {
				gpu_ids[i] = i;
			}
		}

		// Validate given gpu ids
		bool invalid_gpu_ids = false;
		for (const auto gpu_id : gpu_ids) {
			if (gpu_id >= num_devices) {
				std::fprintf(stderr, "[ERROR(gpu_monitor)] GPU %u is not found\n", gpu_id);
				invalid_gpu_ids = true;
			}
		}
		if (invalid_gpu_ids) {
			gpu_monitor.shutdown();
			exit(1);
		}

		// Output csv header
		ofs << "index,date,elapsed_time,";
		for (const auto gpu_id : gpu_ids) {
			ofs << "gpu" << gpu_id << "_temp,";
			ofs << "gpu" << gpu_id << "_power,";
			ofs << "gpu" << gpu_id << "_memory_usage,";
		}
		ofs << "pid,vsize\n";
		ofs.close();

		// record max power, temperature, memory usage
		std::vector<double> max_power(gpu_ids.size());
		for (auto& max_power_v : max_power) max_power_v = 0.0;
		std::vector<double> max_temperature(gpu_ids.size());
		for (auto& max_temperature_v : max_temperature) max_temperature_v = 0.0;
		std::vector<std::size_t> max_memory_usage(gpu_ids.size());
		for (auto& max_memory_usage_v : max_memory_usage) max_memory_usage_v = 0lu;

		std::vector<double> sum_power(gpu_ids.size());
		for (auto& sum_power_v : sum_power) sum_power_v = 0.0;
		std::vector<double> sum_temperature(gpu_ids.size());
		for (auto& sum_temperature_v : sum_temperature) sum_temperature_v = 0.0;
		std::vector<std::size_t> sum_memory_usage(gpu_ids.size());
		for (auto& sum_memory_usage_v : sum_memory_usage) sum_memory_usage_v = 0lu;

		// Output log
		unsigned count = 0;
		const auto start_clock = std::chrono::high_resolution_clock::now();
		do {
			std::ofstream ofs(output_file_name, std::ios::app);
			ofs << (count++) << ","
				<< std::time(nullptr) << ",";
			const auto end_clock = std::chrono::high_resolution_clock::now();
			const auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end_clock - start_clock).count();
			ofs << elapsed_time << ",";
			for (const auto gpu_id : gpu_ids) {
				const auto power = gpu_monitor.get_current_power(gpu_id);
				const auto temperature = gpu_monitor.get_current_temperature(gpu_id);
				const auto memory_usage = gpu_monitor.get_current_used_memory(gpu_id);
				ofs << temperature << ","
					<< power << ","
					<< memory_usage << ",";
				max_power       [gpu_id] = std::max(max_power       [gpu_id], power);
				max_temperature [gpu_id] = std::max(max_temperature [gpu_id], temperature);
				max_memory_usage[gpu_id] = std::max(max_memory_usage[gpu_id], memory_usage);
				sum_power       [gpu_id] += power;
				sum_temperature [gpu_id] += temperature;
				sum_memory_usage[gpu_id] += memory_usage;
			}

			const auto target_pid = *target_pid_ptr;
			ofs << target_pid << ","
				<< get_vsize(target_pid);

			ofs << "\n";
			insert_message(message_file_path, ofs);
			ofs.close();
			const auto end_clock_1 = std::chrono::high_resolution_clock::now();
			const auto elapsed_time_1 = std::chrono::duration_cast<std::chrono::microseconds>(end_clock_1 - start_clock).count();
			usleep(std::max<std::time_t>(time_interval * 1000 * count, elapsed_time_1) - elapsed_time_1);
		} while ((*semaphore) == process::running);

		gpu_monitor.shutdown();

		if (print_result) {
			std::printf(
				"##### GPU Monitoring result #####\n"
				);
			const auto end_clock = std::chrono::high_resolution_clock::now();
			const auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end_clock - start_clock).count() * 1e-6;
			std::printf("- %10s : %.1f [s]\n", "time", elapsed_time);
			for (const auto gpu_id : gpu_ids) {
				std::printf("## ----- GPU %u -----\n", gpu_id);
				std::printf("# Temperature\n");
				std::printf("- %3s : %2.1f [C]\n", "max", max_temperature[gpu_id]);
				std::printf("- %3s : %2.1f [C]\n", "avg", sum_temperature[gpu_id] / count);
				std::printf("# Power\n");
				std::printf("- %3s : %2.1f [W]\n", "max", max_power[gpu_id]);
				std::printf("- %3s : %2.1f [W]\n", "avg", sum_power[gpu_id] / count);
				std::printf("# Memory\n");
				std::printf("- %3s : %.5e [GB]\n", "max", max_memory_usage[gpu_id] / 1e9);
			}
		}

		exit(0);
	} else {
		const auto cmd = argv[run_command_head];
		std::vector<char*> cmd_args(argc - run_command_head + 1);
		for (int i = run_command_head, v = 0; i < argc; i++, v++) {
			cmd_args[v] = argv[i];
		}
		cmd_args[cmd_args.size() - 1] = nullptr;

		const auto command_pid = fork();
		if (command_pid == 0) {
			execvp(cmd, cmd_args.data());
			exit(0);
		} else {
			*target_pid_ptr = command_pid;
			wait(nullptr);
			*semaphore = process::end;
		}
	}

	std::filesystem::remove(message_file_path);
}
