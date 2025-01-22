import numpy as np
file_names=['w_poll_aot_interval.txt',
            'w_poll_aot_interval_sync.txt',
            'w_o_poll_aot_interval_sync.txt',
            'w_poll_interp_interval.txt',
            'w_poll_interp_interval_sync.txt',
            'w_o_poll_interp_interval_sync.txt',
            'new_w_poll_aot_interval.txt',
            'new_w_poll_aot_interval_sync.txt',
            'new_w_o_poll_aot_interval_sync.txt']

for name in file_names:
    failed_times = []
    success_times = []
    with open(name, 'r') as file:
        for line in file:
            line = line.strip()
            if line.startswith("failed,"):
                time = int(line.split(',')[1])
                failed_times.append(time)
            elif line.startswith("success,"):
                time = int(line.split(',')[1])
                success_times.append(time)
            
    failed_times=np.array(failed_times)
    success_times=np.array(success_times)

    print(f'{name} Failed times({len(failed_times)}) mean: {np.mean(failed_times)}')
    print(f'{name} Success times({len(success_times)}) mean: {np.mean(success_times)}')
    print(f'{name} Success rate: {100*len(success_times)/(len(failed_times)+len(success_times))}%')