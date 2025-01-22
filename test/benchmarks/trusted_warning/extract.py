import pandas as pd

# file_paths = ['native.txt', 'interp_sync.txt', 'interp_safe_sync.txt', 'interp_safe_future.txt','interp_safe_callback.txt']
# types = ['native', 'interp watz','interp TrustWasm sync', 'interp TrustWasm async (future)', 'interp TrustWasm async (callback)']
file_paths = ['native.txt', 'aot_sync.txt', 'aot_safe_sync.txt', 'aot_safe_future.txt','aot_safe_callback.txt']
types = ['native', 'aot watz','aot TrustWasm sync', 'aot TrustWasm async (future)', 'aot TrustWasm async (callback)']

all_data = pd.DataFrame()


for file, service_type in zip(file_paths, types):
    data = pd.read_csv(file, header=None, names=['Interval'])
    data.insert(0, 'Type', service_type)
    all_data = pd.concat([all_data, data], ignore_index=True)

# all_data.to_csv('interp_trusted_warning.csv', index=False)
all_data.to_csv('aot_trusted_warning.csv', index=False)