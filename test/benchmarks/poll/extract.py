import pandas as pd

# 读取 CSV 文件
df = pd.read_csv('another_callback.csv')

# 按某一列排序，例如按 'column_name' 排序
df_sorted = df.sort_values(by='Size')

# 如果需要按多个列排序，可以这样做
# df_sorted = df.sort_values(by=['column_name1', 'column_name2'])

# 将排序后的数据保存到新的 CSV 文件
df_sorted.to_csv('size_another_callback.csv', index=False)