#!/bin/bash

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/../wamr-tzio-common.sh

announcebuild "rpi3 gpio access wasm"

build () {
    /opt/wasi-sdk/bin/clang $BM_CFLAGS \
        --target=wasm32-wasi \
        --sysroot=/opt/wasi-sdk/share/wasi-sysroot/ \
        -Wl,--allow-undefined-file=/opt/wasi-sdk/share/wasi-sysroot/share/wasm32-wasi/defined-symbols.txt \
        -Wl,--strip-all \
        -I $SCRIPT_DIR/app \
        -o app/gpio.wasm $SCRIPT_DIR/app/main.c
    
    compileaot $SCRIPT_DIR/app/gpio
}

build

# test.wasm in build/wasm-app
# libiwasm.so, libtest_hello in build/

echo "Waiting on the end of compilation..."
wait

