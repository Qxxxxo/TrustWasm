import pandas as pd
df = pd.read_csv('./w_poll_send_ready_interval.csv')


df["ReadyDelay"]=df["ReadyAt"]-df["SendAt"]
df["HandleDelay"]=df["HandleAt"]-df["ReadyAt"]

avg_times = df.groupby("Result")[["ReadyDelay", "HandleDelay"]].mean()
overall_avg = df[["ReadyDelay", "HandleDelay"]].mean()
overall_avg["Result"] = "overall"
overall_avg_df = pd.DataFrame([overall_avg])
avg_times_with_overall = pd.concat([avg_times.reset_index(), overall_avg_df], ignore_index=True)
print(avg_times_with_overall)

df = pd.read_csv('./w_o_poll_send_ready_interval.csv')


df["ReadyDelay"]=df["ReadyAt"]-df["SendAt"]

avg_times = df.groupby("Result")[["ReadyDelay"]].mean()
overall_avg = df[["ReadyDelay"]].mean()
overall_avg["Result"] = "overall"
overall_avg_df = pd.DataFrame([overall_avg])
avg_times_with_overall = pd.concat([avg_times.reset_index(), overall_avg_df], ignore_index=True)
print(avg_times_with_overall)
