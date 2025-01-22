import pandas as pd
import sys
aot=False
if len(sys.argv)>1:
    if sys.argv[1]=='aot':
         aot=True

file_paths = ['native.txt', 'interp_sync.txt', 'interp_safe_sync.txt', 'interp_safe_future.txt','interp_safe_callback.txt']
types = ['native', 'interp watz','interp TrustWasm sync', 'interp TrustWasm async (future)', 'interp TrustWasm async (callback)']

if aot:
    file_paths = ['native.txt', 'aot_sync.txt', 'aot_safe_sync.txt', 'aot_safe_future.txt','aot_safe_callback.txt']
    types = ['native', 'aot watz','aot TrustWasm sync', 'aot TrustWasm async (future)', 'aot TrustWasm async (callback)']


all_data = pd.DataFrame()


for file, service_type in zip(file_paths, types):
    # 读取数据，假设每行是一个数据
    data = pd.read_csv(file, header=None, names=['Interval'])
    data.insert(0, 'Type', service_type)
    all_data = pd.concat([all_data, data], ignore_index=True)


if aot:
    all_data.to_csv('aot_crop_monitor.csv', index=False)
else:
    all_data.to_csv('interp_crop_monitor.csv', index=False)