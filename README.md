# GPU Logger

This program records GPU temperature, power consumption, used memory while executing some commands.

## Build
```bash
git clone https://github.com/enp1s0/gpu_logger --recursive
cd gpu_logger
make
```

## Usage
```bash
./gpu_logger ./a.out
```

Some options are available to specify time interval and output file name.
```bash
./gpu_logger [-i interval(ms){default=100}] [-o output_file_name{default=gpu.csv}] [-g gpu_id{default=0}] target_command
```

e.g.
```bash
./gpu_logger -i 100 -o report.csv -g 0,2,4 ./a.out
```
