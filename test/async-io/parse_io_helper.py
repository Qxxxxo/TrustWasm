import re
import pandas as pd
ta_log_filename='./tmp_io_helper_ta.txt'
ca_log_filename='./tmp_io_helper.txt'
ta_log_data=[]
ca_log_data=[]

with open(ta_log_filename) as f:
    log_data=f.read()
    pattern = r"check queue start at (\d+), end at (\d+)"
    matches = re.findall(pattern, log_data)
    for start, end in matches:
        start, end = int(start), int(end)
        interval = end - start
        ta_log_data.append({"start_ta": start, "end_ta": end, "interval_ta": interval})
ta_df=pd.DataFrame(ta_log_data)

with open(ca_log_filename) as f:
    log_data=f.read()
    pattern = r"loop start at (\d+), end at (\d+), interval (\d+)"
    matches = re.findall(pattern, log_data)
    for start, end, interval in matches:
        start, end, interval = int(start), int(end), int(interval)
        ca_log_data.append({"start_ca": start, "end_ca": end, "interval_ca": interval})
ca_df=pd.DataFrame(ca_log_data)

df=pd.concat([ta_df,ca_df],axis=1)

df["start_ca_ta"]=df["start_ta"]-df["start_ca"]
df["end_ta_ca"]=df["end_ca"]-df["end_ta"]
df=df[["start_ca_ta","end_ta_ca","interval_ca","interval_ta"]]
print(df.mean())