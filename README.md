# GPU Monitor

This program records GPU temperature, power consumption, memory usage while executing programs on GPUs.
It also provides C++ API.

## Requirements
- C++17 or later
  - This program uses `std::filesystem`

## Installation

Build:
```bash
git clone https://github.com/enp1s0/gpu_monitor --recursive
cd gpu_monitor
mkdir build
cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/install
make -j4
make install
```

Set environment variables:
```
export PATH=/path/to/install/bin:$PATH
# When you use the C++ API, please set following environment variables:
export LIBRARY_PATH=/path/to/install/lib:$LIBRARY_PATH
export LD_LIBRARY_PATH=/path/to/install/lib:$LD_LIBRARY_PATH
export C_INCLUDE_PATH=/path/to/install/include:$C_INCLUDE_PATH
```

## Usage
```bash
gpu_monitor ./a.out
```

Some options are available to specify time interval and output file name.
```bash
gpu_monitor [-i interval(ms){default=100}] [-o output_file_name{default=gpu.csv}] [-g gpu_id{default=0}] target_command
```

e.g.
```bash
gpu_monitor -i 100 -o report.csv -g 0,2,4 ./a.out
```

## Visualization

You can use `scripts/mk_graph.py` to visualize the monitoring result. [detail](./scripts/README.md)

![sample](./docs/gpu.png)

## Insert messages to output file

You can use GPU Monitor API to insert messages to output file.

```cpp
// g++ -std=c++11 -I/path/to/gpu_monitor/include ...
#include <gpu_monitor/gpu_monitor.hpp>

int main() {
    // ...
    mtk::gpu_monitor::insert_message("Hello, world!");
}
```

## C++ Library
This library provides embedded profiling library.
```cpp
// main.cu
// Build: nvcc main.cu -lgpu_monitor -lnvidia-ml ...
#include <gpu_monitor/gpu_monitor.hpp>

void func() {
    const auto profiling_result = mtk::gpu_monitor::measure_power_consumption(
        [&]() {
            // some GPU code
            cudaDeviceSynchronize();
        },
        20, // interval [ms]
    );
}
```

See [sample code](./test/api.cu) for more information.

## License

MIT
