#!/bin/bash

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/../wamr-tzio-common.sh

deploywatz

announcedeploy "native libs ta"
sshpass -p "$BM_BOARD_USER" rsync -e 'ssh -p 22 -o StrictHostKeyChecking=no' --progress $OPTEE_BR_OUT_DIR/build/optee_test_ext-1.0/ta/native_tzio/out/a6799c0f-518a-4107-87be-33757c906262.ta $BM_BOARD_USER@$BM_BOARD_HOSTNAME:/lib/optee_armtz
sshpass -p "$BM_BOARD_USER" ssh $BM_BOARD_USER@$BM_BOARD_HOSTNAME $BM_QEMU_SSH_ARGS 'chmod 666 /lib/optee_armtz/a6799c0f-518a-4107-87be-33757c906262.ta'

announcedeploy "io helper ca"
sshpass -p "$BM_BOARD_USER" rsync -e 'ssh -p 22 -o StrictHostKeyChecking=no' --progress $OPTEE_BR_OUT_DIR/build/optee_examples_ext-1.0/io_helper/io_helper $BM_BOARD_USER@$BM_BOARD_HOSTNAME:/usr/bin
sshpass -p "$BM_BOARD_USER" ssh $BM_BOARD_USER@$BM_BOARD_HOSTNAME $BM_QEMU_SSH_ARGS 'chmod 777 /usr/bin/io_helper'
# pta io helper ca
sshpass -p "$BM_BOARD_USER" rsync -e 'ssh -p 22 -o StrictHostKeyChecking=no' --progress $OPTEE_BR_OUT_DIR/build/optee_examples_ext-1.0/pta_io_helper/pta_io_helper $BM_BOARD_USER@$BM_BOARD_HOSTNAME:/usr/bin
sshpass -p "$BM_BOARD_USER" ssh $BM_BOARD_USER@$BM_BOARD_HOSTNAME $BM_QEMU_SSH_ARGS 'chmod 777 /usr/bin/pta_io_helper'

announcedeploy "io helper ta"
sshpass -p "$BM_BOARD_USER" rsync -e 'ssh -p 22 -o StrictHostKeyChecking=no' --progress $OPTEE_BR_OUT_DIR/build/optee_examples_ext-1.0/io_helper/ta/out/3f4f0011-a4b6-462f-bbae-0678837fcefd.ta $BM_BOARD_USER@$BM_BOARD_HOSTNAME:/lib/optee_armtz
sshpass -p "$BM_BOARD_USER" ssh $BM_BOARD_USER@$BM_BOARD_HOSTNAME $BM_QEMU_SSH_ARGS 'chmod 666 /lib/optee_armtz/3f4f0011-a4b6-462f-bbae-0678837fcefd.ta'

announcedeploy "async-io app"
cd $SCRIPT_DIR
rsync --progress -r $BM_BUILDER_HOSTNAME:$BM_BUILDER_PATH/async_future/app/\*.wasm out/
rsync --progress -r $BM_BUILDER_HOSTNAME:$BM_BUILDER_PATH/async_future/app/\*.aot out/
sshpass -p "$BM_BOARD_USER" rsync -e 'ssh -p 22 -o StrictHostKeyChecking=no' --progress -r out/ $BM_BOARD_USER@$BM_BOARD_HOSTNAME:/root/
