#include <iostream>
#include <gpu_monitor/gpu_monitor.hpp>
#include <unistd.h>

int main() {
	const auto message_file_path = getenv(mtk::gpu_monitor::message_file_path_env_name);
	std::printf("message_file_path = %s\n", message_file_path);

	sleep(1);
	mtk::gpu_monitor::insert_message("hello");
	sleep(1);
	mtk::gpu_monitor::insert_message("world");
	mtk::gpu_monitor::insert_message("!!");
	sleep(1);
}
