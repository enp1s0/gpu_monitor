#ifndef __GPU_MONITOR_H__
#define __GPU_MONITOR_H__
#include <cstdlib>
#include <string>
#include <fstream>
#include <iostream>

namespace mtk {
namespace gpu_monitor {
static const char* message_file_path_env_name = "GLI_MESSGAE_FILE_PATH";
inline void insert_message(const std::string message) {
	const auto message_file_path_ptr = getenv(message_file_path_env_name);
	// If the target program is not launched by gpu_monitor, the length will be zero.
	if (message_file_path_ptr == nullptr) {
		return;
	}
	const std::string message_file_path = message_file_path_ptr;

	std::ofstream ofs(message_file_path, std::ios::app);
	if (!ofs) {
		std::fprintf(stderr, "[gpu_monitor] Could not open messge file %s\n", message_file_path.c_str());
		return;
	}

	ofs << message << "\n";
	ofs.close();
}
} // namespace gpu_monitor
} // namespace mtk

#endif