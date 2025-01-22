import re
import pandas as pd
ta_log_filename='./tmp_get_time.log'
ree_time_intervals=[]
sys_time_intervals=[]

with open(ta_log_filename) as f:
    log_data=f.read()
    pattern=r"interval: (\d+)"
    matches = re.findall(pattern,log_data)
    for interval in matches:
        ree_time_intervals.append(int(interval))
    pattern=r"interval (\d+)"
    matches = re.findall(pattern,log_data)
    for interval in matches:
        sys_time_intervals.append(int(interval))

ree_df=pd.DataFrame(ree_time_intervals)
sys_df=pd.DataFrame(sys_time_intervals)
print(ree_df.mean())
print(sys_df.mean())
