import re
import pandas as pd
ta_log_filename='./tmp_poll_interval_log.txt'
interval_data=[]

with open(ta_log_filename) as f:
    log_data=f.read()
    pattern = r"poll interval: (\d+)"
    matches = re.findall(pattern,log_data)
    for res in matches:
        interval_data.append(int(res))
df=pd.DataFrame(interval_data)
print(df.mean())                                   