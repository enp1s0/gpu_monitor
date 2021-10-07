# GPU Monitor Python extension

This python extension insert messages to the csv log file from Python code.

## Installation
```bash
pip install .
```

## Usage
```python
import gpu_monitor

gpu_monitor.insert_message("Start func A")
# Func A
gpu_monitor.insert_message("End func A")
```
