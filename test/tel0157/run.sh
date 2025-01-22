#!/bin/bash

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/../wamr-tzio-common.sh

mkdir -p $LOGS_DIR/tel0157/
rm -f $LOGS_DIR/tel0157/tel0157.log

wasm_heap_size=$((10 * 1024 * 1024))

announcerun "tel0157"

sshpass -p "$BM_BOARD_USER" ssh $BM_BOARD_USER@$BM_BOARD_HOSTNAME $BM_QEMU_SSH_ARGS "wamr_tzio $wasm_heap_size 1 tel0157.aot 2>&1" | tee  -a $LOGS_DIR/tel0157/tel0157.log
