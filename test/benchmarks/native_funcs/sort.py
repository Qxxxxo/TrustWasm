import pandas as pd

df = pd.read_csv('fix.csv')

df_sorted = df.sort_values(by='Type')

df_sorted.to_csv('fix.csv', index=False)

average_values = df.groupby('Type')['Interval'].mean()

print(average_values)