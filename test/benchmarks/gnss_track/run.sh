#!/bin/bash

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/../../wamr-tzio-common.sh

mkdir -p $LOGS_DIR/gnss_track/
rm -f $LOGS_DIR/gnss_track/gnss_track.log

wasm_heap_size=$((10 * 1024 * 1024))

announcerun "gnss_track"

sshpass -p "$BM_BOARD_USER" ssh $BM_BOARD_USER@$BM_BOARD_HOSTNAME $BM_QEMU_SSH_ARGS "wamr_tzio $wasm_heap_size 1 gnss_track.$1 2>&1" | tee  -a $LOGS_DIR/gnss_track/gnss_track.log
