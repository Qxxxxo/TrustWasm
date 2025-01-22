#!/bin/bash

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/../wamr-tzio-common.sh

announcebuild "wasm bme280"

build () {
    /opt/wasi-sdk/bin/clang $BM_CFLAGS \
        --target=wasm32-wasi \
        --sysroot=/opt/wasi-sdk/share/wasi-sysroot/ \
        -Wl,--allow-undefined-file=/opt/wasi-sdk/share/wasi-sysroot/share/wasm32-wasi/defined-symbols.txt \
        -Wl,--strip-all \
        -I $SCRIPT_DIR/app \
        -o app/bme280.wasm $SCRIPT_DIR/app/main.c
    
    compileaot $SCRIPT_DIR/app/bme280
}

SOMETHING=main.c

build_something (){
    echo "Compile file: $SOMETHING"
    /opt/wasi-sdk/bin/clang $BM_CFLAGS \
        --target=wasm32-wasi \
        --sysroot=/opt/wasi-sdk/share/wasi-sysroot/ \
        -Wl,--allow-undefined-file=/opt/wasi-sdk/share/wasi-sysroot/share/wasm32-wasi/defined-symbols.txt \
        -Wl,--strip-all \
        -I $SCRIPT_DIR/app \
        -o app/bme280.wasm $SCRIPT_DIR/app/$SOMETHING
    
    compileaot $SCRIPT_DIR/app/bme280
}

if [ "$1" == "main" ]; then 
    build
else 
    SOMETHING=$1
    build_something
fi

echo "Waiting on the end of compilation..."
wait

