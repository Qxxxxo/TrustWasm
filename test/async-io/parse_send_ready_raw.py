import re
import pandas as pd
ta_log_filename='./w_o_poll_send_ready_log.txt'
ta_log_data=[]
wasm_log_data=[]

with open(ta_log_filename) as f:
    log_data=f.read()
    pattern = r"handle a req with result (-?\d+) at (\d+)"
    matches = re.findall(pattern, log_data)
    for res,end in matches:
        res, end = int(res), int(end)
        ta_log_data.append({"result": "success" if res==0 else "failed", "end": end})
    pattern = r"start time: (\d+),"
    matches = re.findall(pattern,log_data)
    for start in matches:
        wasm_log_data.append({"start":int(start)})
ta_df=pd.DataFrame(ta_log_data)
wasm_df=pd.DataFrame(wasm_log_data)
df=pd.concat([ta_df,wasm_df],axis=1)
df["ready_delay"]=df["end"]-df["start"]
print(df["ready_delay"].mean())

ta_log_filename='./w_poll_send_ready_log.txt'
ta_log_data=[]
wasm_log_data=[]

with open(ta_log_filename) as f:
    log_data=f.read()
    pattern = r"handle a req with result (-?\d+) at (\d+)"
    matches = re.findall(pattern, log_data)
    for res,end in matches:
        res, end = int(res), int(end)
        ta_log_data.append({"result": "success" if res==0 else "failed", "req_end": end})
    pattern = r"start time: (\d+), end time: (\d+), interval: (\d+)"
    matches = re.findall(pattern,log_data)
    for start,end,interval in matches:
        wasm_log_data.append({"start":int(start),"handle_end":int(end),"interval":int(interval)})
ta_df=pd.DataFrame(ta_log_data)
wasm_df=pd.DataFrame(wasm_log_data)
df=pd.concat([ta_df,wasm_df],axis=1)
df["ready_delay"]=df["req_end"]-df["start"]
df["handle_delay"]=df["handle_end"]-df["req_end"]
print(df[["ready_delay","handle_delay"]].mean())