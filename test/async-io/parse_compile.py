import numpy as np
w_poll_compile_filename='w_poll_compile.txt'
w_o_poll_compile_filename='w_o_poll_compile.txt'
w_poll_compile_sync_filename='w_poll_compile_sync.txt'
w_o_poll_compile_sync_filename='w_o_poll_compile_sync.txt'
w_poll_compile_times = []
w_o_poll_compile_times =[]
w_poll_compile_sync_times = []
w_o_poll_compile_sync_times = []

with open(w_poll_compile_filename, 'r') as file:
    for line in file:
        line = line.strip()
        w_poll_compile_times.append(int(line))

with open(w_o_poll_compile_filename, 'r') as file:
    for line in file:
        line = line.strip()
        w_o_poll_compile_times.append(int(line))

with open(w_poll_compile_sync_filename, 'r') as file:
    for line in file:
        line = line.strip()
        w_poll_compile_sync_times.append(int(line))

with open(w_o_poll_compile_sync_filename, 'r') as file:
    for line in file:
        line = line.strip()
        w_o_poll_compile_sync_times.append(int(line))

w_poll_compile_times=np.array(w_poll_compile_times)
w_o_poll_compile_times=np.array(w_o_poll_compile_times)
w_poll_compile_sync_times=np.array(w_poll_compile_sync_times)
w_o_poll_compile_sync_times=np.array(w_o_poll_compile_sync_times)
print(f'With poll compile times ({len(w_poll_compile_times)}) mean: {np.mean(w_poll_compile_times)} ms')
print(f'Without poll compile times ({len(w_o_poll_compile_times)}) mean: {np.mean(w_o_poll_compile_times)} ms')
print(f'Without poll compile times ({len(w_poll_compile_sync_times)}) mean: {np.mean(w_poll_compile_sync_times)} ms')
print(f'Without poll compile times ({len(w_o_poll_compile_sync_times)}) mean: {np.mean(w_o_poll_compile_sync_times)} ms')