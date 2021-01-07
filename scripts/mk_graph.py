import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import pandas as pd
import os

input_name = 'gpu.csv'
output_name = 'gpu.pdf'
gpu_ids = [0]

def draw_memory_usage_graph(ax, gpu_ids):
    ax.grid()
    ax.set_xlabel('time [s]')
    ax.set_ylabel("memory usage [GiB]")
    ax.set_facecolor('white')

    df = pd.read_csv(input_name)
    line_list = []
    label_list = []
    for gpu_id in gpu_ids:
        l = ax.plot(df['elapsed_time'] / 1000000, df['gpu' + str(gpu_id) + '_memory_usage'] / (1024*1024*1024), linewidth=2, marker='*')
        line_list += [l]
        label_list += ['GPU ' + str(gpu_id)]
    return line_list, label_list

def draw_power_graph(ax, gpu_ids):
    ax.grid()
    ax.set_xlabel('time [s]')
    ax.set_ylabel("power [W]")
    ax.set_facecolor('white')

    df = pd.read_csv(input_name)
    line_list = []
    label_list = []
    for gpu_id in gpu_ids:
        l = ax.plot(df['elapsed_time'] / 1000000, df['gpu' + str(gpu_id) + '_power'], linewidth=2, marker='*')
        line_list += [l]
        label_list += ['GPU ' + str(gpu_id)]
    return line_list, label_list

def draw_temperature_graph(ax, gpu_ids):
    ax.grid()
    ax.set_xlabel('time [s]')
    ax.set_ylabel("temperature [C]")
    ax.set_facecolor('white')

    df = pd.read_csv(input_name)
    line_list = []
    label_list = []
    for gpu_id in gpu_ids:
        l = ax.plot(df['elapsed_time'] / 1000000, df['gpu' + str(gpu_id) + '_temp'], linewidth=2, marker='*')
        line_list += [l]
        label_list += ['GPU ' + str(gpu_id)]
    return line_list, label_list

fig, ((ax0, ax1, ax2)) = plt.subplots(1, 3, figsize=(12, 3))

draw_temperature_graph(ax0, gpu_ids)
draw_power_graph(ax1, gpu_ids)
line_list, label_list = draw_memory_usage_graph(ax2, gpu_ids)

fig.legend(line_list,
        labels=label_list,
        loc='upper center',
        ncol=1,
        bbox_to_anchor=(0.8, 1.2),
        bbox_transform=ax1.transAxes
        )
plt.tight_layout()

plt.savefig(output_name, bbox_inches="tight", transparent=True)
