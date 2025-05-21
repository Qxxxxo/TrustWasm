#!/bin/bash

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/../../wamr-tzio-common.sh

mkdir -p $LOGS_DIR/trusted_warning_power/
rm -f $LOGS_DIR/trusted_warning_power/trusted_warning_power.log

wasm_heap_size=$((10 * 1024 * 1024))

announcerun "trusted_warning_power"

sshpass -p "$BM_BOARD_USER" ssh $BM_BOARD_USER@$BM_BOARD_HOSTNAME $BM_QEMU_SSH_ARGS "wamr_tzio $wasm_heap_size 1 trusted_warning_power.$1 2>&1" | tee  -a $LOGS_DIR/trusted_warning_power/trusted_warning_power.log
