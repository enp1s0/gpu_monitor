#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <vector>
#include <exception>
#include <unistd.h>
#include <cutf/device.hpp>
#include <cutf/nvml.hpp>

void parse_params(unsigned &time_interval, std::string& output_file_name, int& run_command_head, int argc, char** argv) {
	run_command_head = 1;
	output_file_name = "gpu.log";
	time_interval = 1;
	for (int i = 1; i < argc; i++) {
		if (std::string(argv[i]) == "-i") {
			if (i + 1 >= argc) {
				throw std::runtime_error("The value of `-i` is not provided");
			}
			time_interval = std::stoul(argv[i+1]);
			i += 2;
		} else if (std::string(argv[i]) == "-i") {
			if (i + 1 >= argc) {
				throw std::runtime_error("The value of `-o` is not provided");
			}
			output_file_name = argv[i+1];
			i += 2;
		} else {
			run_command_head = i;
			return;
		}
	}
}

int main(int argc, char** argv) {
	std::string output_file_name;
	unsigned time_interval;
	int run_command_head;

	parse_params(time_interval, output_file_name, run_command_head, argc, argv);

	int pid = fork();
	if (pid != 0) {
		std::ofstream ofs(output_file_name);
		CUTF_CHECK_ERROR(nvmlInit());
		const auto num_devices = cutf::device::get_num_devices();

		// Output csv header
		ofs << "index,date,";
		for (unsigned gpu_id = 0; gpu_id < num_devices; gpu_id++) {
			ofs << "gpu" << gpu_id << "_temp,";
			ofs << "gpu" << gpu_id << "_power,";
			ofs << "gpu" << gpu_id << "_memory,";
		}
		ofs << "\n";
		ofs.close();

		// Output log
		unsigned count = 0;
		while (1) {
			std::ofstream ofs(output_file_name, std::ios::app);
			ofs << std::time(nullptr) << ","
				<< (count++) << ",";
			for (unsigned gpu_id = 0; gpu_id < num_devices; gpu_id++) {
				nvmlDevice_t device;
				CUTF_CHECK_ERROR(nvmlDeviceGetHandleByIndex(gpu_id, &device));

				unsigned int temperature;
				CUTF_CHECK_ERROR(nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &temperature));
				unsigned int memory;
				CUTF_CHECK_ERROR(nvmlDeviceGetMemoryInfo(device, memory));
				unsigned int power;
				CUTF_CHECK_ERROR(nvmlDeviceGetPowerUsage(device, power));

				ofs << temperature << ","
					<< power << ","
					<< memory << ",";
			}
			ofs.close();
			sleep(time_interval * 1000);
		}

		CUTF_CHECK_ERROR(nvmlShutdown());
	} else {
		const auto cmd = argv[run_command_head];
		std::vector<char*> cmd_args(argc - run_command_head + 1);	
		for (int i = 0, v = 0; i < run_command_head; i++, v++) {
			cmd_args[v] = argv[i];
		}
		cmd_args[cmd_args.size() - 1] = nullptr;

		execvp(cmd, cmd_args.data());
		exit(0);
	}
}
