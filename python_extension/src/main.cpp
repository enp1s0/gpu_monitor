#include <pybind11/pybind11.h>
#include "../../include/gpu_monitor/gpu_monitor.hpp"

void insert_message(const std::string message) {
	mtk::gpu_monitor::insert_message(message);
}

PYBIND11_MODULE(gpu_monitor, m) {
	m.doc() = "GPU Monitor python extension";
	m.def("insert_message", &insert_message, "A function that inserts message to a log file");
}
