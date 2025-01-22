#!/bin/bash

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/../wamr-tzio-common.sh

mkdir -p $LOGS_DIR/mpu6050/
rm -f $LOGS_DIR/mpu6050/mpu6050.log

wasm_heap_size=$((10 * 1024 * 1024))

announcerun "mpu6050"

sshpass -p "$BM_BOARD_USER" ssh $BM_BOARD_USER@$BM_BOARD_HOSTNAME $BM_QEMU_SSH_ARGS "wamr_tzio $wasm_heap_size 1 mpu6050.aot 2>&1" | tee  -a $LOGS_DIR/mpu6050/mpu6050.log
