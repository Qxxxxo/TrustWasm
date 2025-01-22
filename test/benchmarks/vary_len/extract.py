import pandas as pd


df = pd.read_csv('another_callback.csv')

df_sorted = df.sort_values(by='Size')

# df_sorted = df.sort_values(by=['column_name1', 'column_name2'])

df_sorted.to_csv('size_another_callback.csv', index=False)