#!/bin/bash

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/../wamr-tzio-common.sh

announcebuild "wasm dht"

build () {
    /opt/wasi-sdk/bin/clang $BM_CFLAGS \
        --target=wasm32-wasi \
        --sysroot=/opt/wasi-sdk/share/wasi-sysroot/ \
        -Wl,--allow-undefined-file=/opt/wasi-sdk/share/wasi-sysroot/share/wasm32-wasi/defined-symbols.txt \
        -Wl,--strip-all \
        -I $SCRIPT_DIR/app \
        -o app/dht.wasm $SCRIPT_DIR/app/main.c
        # -o app/dht.wasm $SCRIPT_DIR/app/sync.c
        # -o app/dht.wasm $SCRIPT_DIR/app/main.c
        # -o app/dht.wasm $SCRIPT_DIR/app/time_overhead.c
        # -o app/dht.wasm $SCRIPT_DIR/app/sync.c
    compileaot $SCRIPT_DIR/app/dht
    # compile wasm on board now
}

build

echo "Waiting on the end of compilation..."
wait

