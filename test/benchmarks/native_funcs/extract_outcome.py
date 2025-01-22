import pandas as pd
import re

with open('./sync_fix.txt', 'r') as file:
    lines = file.readlines()

pattern = r'res\s+\w+,\s+(.+?)\s+interval\s+(\d+)' 

intervals = []
gpio_set_count = 0
for line in lines:
    match = re.search(pattern, line)
    if match:
        function_name = match.group(1).strip()
        if function_name.endswith("init") or function_name in ["gpio_set", "gpio_get"]:
            continue
        interval_value = int(match.group(2))
        intervals.append({"Function": function_name, "Interval": interval_value})

df = pd.DataFrame(intervals)

df = df.sort_values(by="Function")

print(df)

df.to_csv('sync_fix.csv', index=False)