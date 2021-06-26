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

void parse_params(unsigned &time_interval, std::string& output_file_name, std::vector<unsigned>& gpu_ids, int& run_command_head, int argc, char** argv) {
	run_command_head = 1;
	output_file_name = "gpu.csv";
	time_interval = 100;
	gpu_ids = std::vector<unsigned>{0};
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
			gpu_ids = get_gpu_ids(argv[i+1]);
			i += 2;
		} else if (std::string(argv[i]) == "-h") {
			time_interval = 0; // This means that this execution is invalid and exits with printing help messages.
		} else {
			run_command_head = i;
			return;
		}
	}
}

void print_help_message(const char* const program_name) {
	std::printf("/*** GPU Logger ***/\n");
	std::printf("\n");
	std::printf("// Usage\n");
	std::printf("%s [-i interval(ms){default=100}] [-o output_file_name{default=gpu.csv}] [-g gpu_id{default=0}] target_command\n", program_name);
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

	parse_params(time_interval, output_file_name, gpu_ids, run_command_head, argc, argv);

	if (time_interval < 1 || argc <= 1) {
		print_help_message(argv[0]);
		return 1;
	}

	const auto fd = shm_open("/gpu_monitor_smem", O_CREAT | O_RDWR, 0666);
	ftruncate(fd, 1);
	const auto semaphore = static_cast<char*>(mmap(nullptr, 1, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
	*semaphore = process::running;

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
		ofs << "\n";
		ofs.close();

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
				ofs << gpu_monitor.get_current_temperature(gpu_id) << ","
					<< gpu_monitor.get_current_power(gpu_id) << ","
					<< gpu_monitor.get_current_used_memory(gpu_id) << ",";
			}
			ofs << "\n";
			insert_message(message_file_path, ofs);
			ofs.close();
			const auto end_clock_1 = std::chrono::high_resolution_clock::now();
			const auto elapsed_time_1 = std::chrono::duration_cast<std::chrono::microseconds>(end_clock_1 - start_clock).count();
			usleep(std::max<std::time_t>(time_interval * 1000 * count, elapsed_time_1) - elapsed_time_1);
		} while ((*semaphore) == process::running);

		gpu_monitor.shutdown();

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
			wait(nullptr);
			*semaphore = process::end;
		}
	}

	std::filesystem::remove(message_file_path);
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
		// trapeziodal integration
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
