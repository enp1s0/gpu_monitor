# GPU Logger

This program records GPU temperature, power consumption, used memory while executing some commands.

## Build
```bash
git clone https://github.com/enp1s0/gpu_logger --recursive
cd gpu_logger
make
```

## Usage
```
./gpu_logger [-i interval(s){default=1}] [-o output_file_name{default=gpu.csv}] target_command
```
