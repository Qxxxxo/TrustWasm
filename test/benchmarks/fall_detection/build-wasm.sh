#!/bin/bash

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/../../wamr-tzio-common.sh

announcebuild "wasm fall_detection"

build_safe_sync () {
    /opt/wasi-sdk/bin/clang $BM_CFLAGS \
        --target=wasm32-wasi \
        --sysroot=/opt/wasi-sdk/share/wasi-sysroot/ \
        -Wl,--allow-undefined-file=/opt/wasi-sdk/share/wasi-sysroot/share/wasm32-wasi/defined-symbols.txt \
        -Wl,--strip-all \
        -I $SCRIPT_DIR/app \
        -o app/fall_detection.wasm $SCRIPT_DIR/app/safe_sync.c
    
    compileaot $SCRIPT_DIR/app/fall_detection
}

build_safe_async () {
    /opt/wasi-sdk/bin/clang $BM_CFLAGS \
        --target=wasm32-wasi \
        --sysroot=/opt/wasi-sdk/share/wasi-sysroot/ \
        -Wl,--allow-undefined-file=/opt/wasi-sdk/share/wasi-sysroot/share/wasm32-wasi/defined-symbols.txt \
        -Wl,--strip-all \
        -I $SCRIPT_DIR/app \
        -o app/fall_detection.wasm $SCRIPT_DIR/app/safe_async.c
    
    compileaot $SCRIPT_DIR/app/fall_detection
}

build_sync () {
    /opt/wasi-sdk/bin/clang $BM_CFLAGS \
        --target=wasm32-wasi \
        --sysroot=/opt/wasi-sdk/share/wasi-sysroot/ \
        -Wl,--allow-undefined-file=/opt/wasi-sdk/share/wasi-sysroot/share/wasm32-wasi/defined-symbols.txt \
        -Wl,--strip-all \
        -I $SCRIPT_DIR/app \
        -o app/fall_detection.wasm $SCRIPT_DIR/app/sync.c
    
    compileaot $SCRIPT_DIR/app/fall_detection
}

build_async () {
    /opt/wasi-sdk/bin/clang $BM_CFLAGS \
        --target=wasm32-wasi \
        --sysroot=/opt/wasi-sdk/share/wasi-sysroot/ \
        -Wl,--allow-undefined-file=/opt/wasi-sdk/share/wasi-sysroot/share/wasm32-wasi/defined-symbols.txt \
        -Wl,--strip-all \
        -I $SCRIPT_DIR/app \
        -o app/fall_detection.wasm $SCRIPT_DIR/app/async.c
    
    compileaot $SCRIPT_DIR/app/fall_detection
}

SOMETHING=sync.c

build_something (){
    echo "Compile file: $SOMETHING"
    /opt/wasi-sdk/bin/clang $BM_CFLAGS \
        --target=wasm32-wasi \
        --sysroot=/opt/wasi-sdk/share/wasi-sysroot/ \
        -Wl,--allow-undefined-file=/opt/wasi-sdk/share/wasi-sysroot/share/wasm32-wasi/defined-symbols.txt \
        -Wl,--strip-all \
        -I $SCRIPT_DIR/app \
        -o app/fall_detection.wasm $SCRIPT_DIR/app/$SOMETHING
    
    compileaot $SCRIPT_DIR/app/fall_detection
}

if [ "$1" == "safe_async" ]; then
    build_safe_async
elif [ "$1" == "safe_sync" ]; then    
    build_safe_sync
elif [ "$1" == "async" ]; then
    build_async
elif [ "$1" == "sync" ]; then   
    build_sync
else
    SOMETHING=$1
    build_something
fi

echo "Waiting on the end of compilation..."
wait

