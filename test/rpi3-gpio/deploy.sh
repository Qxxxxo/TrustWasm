#!/bin/bash

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/../wamr-tzio-common.sh

deploywatz

announcedeploy "native libs ta"
sshpass -p "$BM_BOARD_USER" rsync -e 'ssh -p 22 -o StrictHostKeyChecking=no' --progress $OPTEE_BR_OUT_DIR/build/optee_test_ext-1.0/ta/native_lib/out/b3091a65-9751-4784-abf7-0298a7cc35bb.ta $BM_BOARD_USER@$BM_BOARD_HOSTNAME:/lib/optee_armtz
sshpass -p "$BM_BOARD_USER" ssh $BM_BOARD_USER@$BM_BOARD_HOSTNAME $BM_QEMU_SSH_ARGS 'chmod 666 /lib/optee_armtz/b3091a65-9751-4784-abf7-0298a7cc35bb.ta'
sshpass -p "$BM_BOARD_USER" rsync -e 'ssh -p 22 -o StrictHostKeyChecking=no' --progress $OPTEE_BR_OUT_DIR/build/optee_test_ext-1.0/ta/native_lib_gpio/out/b3091a65-9751-4784-abf7-0298a7cc35bc.ta $BM_BOARD_USER@$BM_BOARD_HOSTNAME:/lib/optee_armtz
sshpass -p "$BM_BOARD_USER" ssh $BM_BOARD_USER@$BM_BOARD_HOSTNAME $BM_QEMU_SSH_ARGS 'chmod 666 /lib/optee_armtz/b3091a65-9751-4784-abf7-0298a7cc35bc.ta'

announcedeploy "rpi3-gpio"
cd $SCRIPT_DIR
rsync --progress -r $BM_BUILDER_HOSTNAME:$BM_BUILDER_PATH/rpi3-gpio/app/\*.aot out/
sshpass -p "$BM_BOARD_USER" rsync -e 'ssh -p 22 -o StrictHostKeyChecking=no' --progress -r out/ $BM_BOARD_USER@$BM_BOARD_HOSTNAME:/root/