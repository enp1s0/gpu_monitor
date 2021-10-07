# GPU Monitor Python extension

This python extension insert messages to the csv log file from Python code.

## Installation
```bash
pip install -r requirements.txt
pip install .
```

## Usage
```python
import gpu_monitor

gpu_monitor.insert_message("Start func A")
# Func A
gpu_monitor.insert_message("End func A")
```

## Test
```bash
gpu_monitor python tests/main.py
```
