#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <vector>
#include <exception>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cutf/device.hpp>
#include <cutf/nvml.hpp>

void parse_params(unsigned &time_interval, std::string& output_file_name, int& run_command_head, int argc, char** argv) {
	run_command_head = 1;
	output_file_name = "gpu.log";
	time_interval = 1;
	for (int i = 1; i < argc;) {
		if (std::string(argv[i]) == "-i") {
			if (i + 1 >= argc) {
				throw std::runtime_error("The value of `-i` is not provided");
			}
			time_interval = std::stoul(argv[i+1]);
			i += 2;
		} else if (std::string(argv[i]) == "-o") {
			if (i + 1 >= argc) {
				throw std::runtime_error("The value of `-o` is not provided");
			}
			output_file_name = argv[i+1];
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
	std::printf("%s [-i interval(s){default=1}] [-o output_file_name{default=gpu.csv}] target_command\n", program_name);
}

namespace process {
constexpr char running = 'R';
constexpr char end     = 'E';
} // namespace process

int main(int argc, char** argv) {
	std::string output_file_name;
	unsigned time_interval;
	int run_command_head;

	parse_params(time_interval, output_file_name, run_command_head, argc, argv);

	if (time_interval < 1 || argc <= 1) {
		print_help_message(argv[0]);
		return 1;
	}

	const auto fd = shm_open("/gpu_logger_smem", O_CREAT | O_RDWR, 0666);
	ftruncate(fd, 1);
	const auto semaphore = static_cast<char*>(mmap(nullptr, 1, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
	*semaphore = process::running;

	const auto pid = fork();
	if (pid == 0) {
		std::ofstream ofs(output_file_name);
		CUTF_CHECK_ERROR(nvmlInit());
		const auto num_devices = cutf::device::get_num_devices();

		// Output csv header
		ofs << "index,date,";
		for (unsigned gpu_id = 0; gpu_id < num_devices; gpu_id++) {
			ofs << "gpu" << gpu_id << "_temp,";
			ofs << "gpu" << gpu_id << "_power,";
			ofs << "gpu" << gpu_id << "_memory_used,";
		}
		ofs << "\n";
		ofs.close();

		// Output log
		unsigned count = 0;
		while ((*semaphore) == process::running) {
			std::ofstream ofs(output_file_name, std::ios::app);
			ofs << (count++) << ","
				<< std::time(nullptr) << ",";
			for (unsigned gpu_id = 0; gpu_id < num_devices; gpu_id++) {
				nvmlDevice_t device;
				CUTF_CHECK_ERROR(nvmlDeviceGetHandleByIndex(gpu_id, &device));

				unsigned int temperature;
				CUTF_CHECK_ERROR(nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &temperature));
				nvmlMemory_t memory;
				CUTF_CHECK_ERROR(nvmlDeviceGetMemoryInfo(device, &memory));
				unsigned int power;
				CUTF_CHECK_ERROR(nvmlDeviceGetPowerUsage(device, &power));

				ofs << temperature << ","
					<< (power / 1000.0) << ","
					<< memory.used << ",";
			}
			ofs << "\n";
			ofs.close();
			sleep(time_interval);
		}

		CUTF_CHECK_ERROR(nvmlShutdown());
	} else {
		const auto cmd = argv[run_command_head];
		std::vector<char*> cmd_args(argc - run_command_head + 1);	
		for (int i = run_command_head, v = 0; i < argc; i++, v++) {
			cmd_args[v] = argv[i];
		}
		cmd_args[cmd_args.size() - 1] = nullptr;

		execvp(cmd, cmd_args.data());
		*semaphore = process::end;
		exit(0);
	}
}
