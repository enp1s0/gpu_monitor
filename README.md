# GPU Monitor

This program records GPU temperature, power consumption, used memory while executing some commands.

## Requirements
- C++17 or later
  - This program uses `std::filesystem`

## Build
```bash
git clone https://github.com/enp1s0/gpu_monitor --recursive
cd gpu_monitor
make
```

## Usage
```bash
./gpu_monitor ./a.out
```

Some options are available to specify time interval and output file name.
```bash
./gpu_monitor [-i interval(ms){default=100}] [-o output_file_name{default=gpu.csv}] [-g gpu_id{default=0}] target_command
```

e.g.
```bash
./gpu_monitor -i 100 -o report.csv -g 0,2,4 ./a.out
```

## Insert messages to output file

You can use GPU Monitor API to insert messages to output file.

```cpp
// g++ -std=c++11 -I/path/to/gpu_monitor/include ...
#include <gpu_monito/gpu_monitor.h>

int main() {
    // ...
    mtk::gpu_monitor::insert_message("Hello, world!");
}
```

## License

MIT
