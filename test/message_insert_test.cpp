#include <iostream>
#include <gpu_monitor/gpu_monitor.h>
#include <unistd.h>

int main() {
	const auto message_file_path = getenv(mtk::gpu_monitor::message_file_path_env_name);
	std::printf("message_file_path = %s\n", message_file_path);

	mtk::gpu_monitor::insert_message("hello");
	sleep(1);
	mtk::gpu_monitor::insert_message("world");
	sleep(1);
}
