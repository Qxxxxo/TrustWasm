#!/bin/bash

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/../../wamr-tzio-common.sh

mkdir -p $LOGS_DIR/vary_freq/
rm -f $LOGS_DIR/vary_freq/vary_freq.log

wasm_heap_size=$((10 * 1024 * 1024))

announcerun "vary_freq"

sshpass -p "$BM_BOARD_USER" ssh $BM_BOARD_USER@$BM_BOARD_HOSTNAME $BM_QEMU_SSH_ARGS "wamr_tzio $wasm_heap_size 1 vary_freq.$1 2>&1" | tee  -a $LOGS_DIR/vary_freq/vary_freq.log
