#!/bin/bash

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/../wamr-tzio-common.sh

mkdir -p $LOGS_DIR/async_promise
rm -f $LOGS_DIR/async_promise/async_promise.log

wasm_heap_size=$((10 * 1024 * 1024))

announcerun "dht"

# sshpass -p "$BM_BOARD_USER" ssh $BM_BOARD_USER@$BM_BOARD_HOSTNAME $BM_QEMU_SSH_ARGS "wamr_tzio $wasm_heap_size 1 dht.wasm 2>&1" | tee  -a $LOGS_DIR/async_promise/async_promise.log
sshpass -p "$BM_BOARD_USER" ssh $BM_BOARD_USER@$BM_BOARD_HOSTNAME $BM_QEMU_SSH_ARGS "wamr_tzio $wasm_heap_size 1 async_promise.aot 2>&1" | tee  -a $LOGS_DIR/async_promise/async_promise.log
