#!/bin/bash

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/../wamr-tzio-common.sh

deploywatz

announcedeploy "native libs ta"
sshpass -p "$BM_BOARD_USER" rsync -e 'ssh -p 22 -o StrictHostKeyChecking=no' --progress $OPTEE_BR_OUT_DIR/build/optee_test_ext-1.0/ta/native_tzio/out/a6799c0f-518a-4107-87be-33757c906262.ta $BM_BOARD_USER@$BM_BOARD_HOSTNAME:/lib/optee_armtz
sshpass -p "$BM_BOARD_USER" ssh $BM_BOARD_USER@$BM_BOARD_HOSTNAME $BM_QEMU_SSH_ARGS 'chmod 666 /lib/optee_armtz/a6799c0f-518a-4107-87be-33757c906262.ta'

announcedeploy "wasm bme280"
cd $SCRIPT_DIR
rsync --progress -r $BM_BUILDER_HOSTNAME:$BM_BUILDER_PATH/bme280/app/\*.aot out/
sshpass -p "$BM_BOARD_USER" rsync -e 'ssh -p 22 -o StrictHostKeyChecking=no' --progress -r out/ $BM_BOARD_USER@$BM_BOARD_HOSTNAME:/root/