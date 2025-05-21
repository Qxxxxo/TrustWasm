#!/bin/bash

# 在后台运行即使关闭终端也不会停止
nohup ./run.sh aot > $1.log 2>&1 &

# e.g.
# ./record_run.sh sync