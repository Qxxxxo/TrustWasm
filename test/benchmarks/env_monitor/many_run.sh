./build.sh $1
./deploy.sh
for i in {1..2}; do
    ./run.sh | grep -a "interval" | sed 's/.*interval \([0-9]*\).*/\1/' >> $1.txt
done