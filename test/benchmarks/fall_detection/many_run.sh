for i in {1..100}; do
  # ./build.sh | grep "Compile end" | grep "total time" | awk -F'total time: ' '{print $2}' | awk '{print $1}' >> w_o_poll_compile_sync.txt
  # ./build.sh | grep "Compile end" | grep "total time" | awk -F'total time: ' '{print $2}' | awk '{print $1}' >> w_poll_compile_sync.txt
  # ./build.sh | grep "Compile end" | grep "total time" | awk -F'total time: ' '{print $2}' | awk '{print $1}' >> w_poll_compile.txt
  # ./run.sh | grep -oP 'failed|\bsuccess\b|interval: \K\d+' | tr '\n' ',' | sed 's/,$/\n/' >> w_poll_aot_interval.txt
  # ./run.sh | grep -oP 'failed|\bsuccess\b|interval: \K\d+' | tr '\n' ',' | sed 's/,$/\n/' >> w_poll_aot_interval_sync.txt
  # ./run.sh | grep -oP 'failed|\bsuccess\b|interval: \K\d+' | tr '\n' ',' | sed 's/,$/\n/' >> w_o_poll_aot_interval_sync.txt
  # ./run.sh | grep -oP 'failed|\bsuccess\b|interval: \K\d+' | tr '\n' ',' | sed 's/,$/\n/' >> w_poll_interp_interval.txt
  # ./run.sh | grep -oP 'failed|\bsuccess\b|interval: \K\d+' | tr '\n' ',' | sed 's/,$/\n/' >> w_poll_interp_interval_sync.txt
  # ./run.sh | grep -oP 'failed|\bsuccess\b|interval: \K\d+' | tr '\n' ',' | sed 's/,$/\n/' >> w_o_poll_interp_interval_sync.txt
  # ./run.sh
  #  ./run.sh | grep -oP 'failed|\bsuccess\b|interval: \K\d+' | tr '\n' ',' | sed 's/,$/\n/' >> new_w_poll_aot_interval.txt
  # ./run.sh | grep -oP 'failed|\bsuccess\b|interval: \K\d+' | tr '\n' ',' | sed 's/,$/\n/' >> new_w_poll_aot_interval_sync.txt
#   ./run.sh | grep -a "interval" $1 | sed 's/.*interval \([0-9]*\).*/\1/' >> safe_async_50.txt
#   ./run.sh | grep -a "interval" $1 | sed 's/.*interval \([0-9]*\).*/\1/' >> safe_sync_50.txt
#   ./run.sh | grep -a "interval" $1 | sed 's/.*interval \([0-9]*\).*/\1/' >> async_50.txt
  ./run.sh | grep -a "interval" $1 | sed 's/.*interval \([0-9]*\).*/\1/' >> safe_sync_math_50.txt
done