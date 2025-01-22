#!/bin/bash

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/../../wamr-tzio-common.sh

mkdir -p $LOGS_DIR/native_funcs/
rm -f $LOGS_DIR/native_funcs/native_funcs.log

wasm_heap_size=$((10 * 1024 * 1024))

announcerun "native_funcs"

sshpass -p "$BM_BOARD_USER" ssh $BM_BOARD_USER@$BM_BOARD_HOSTNAME $BM_QEMU_SSH_ARGS "wamr_tzio $wasm_heap_size 1 native_funcs.aot 2>&1" | tee  -a $LOGS_DIR/native_funcs/native_funcs.log
