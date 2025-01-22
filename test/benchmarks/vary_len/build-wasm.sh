#!/bin/bash

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/../../wamr-tzio-common.sh

announcebuild "wasm vary_len"


build_safe_sync () {
    /opt/wasi-sdk/bin/clang $BM_CFLAGS \
        --target=wasm32-wasi \
        --sysroot=/opt/wasi-sdk/share/wasi-sysroot/ \
        -Wl,--allow-undefined-file=/opt/wasi-sdk/share/wasi-sysroot/share/wasm32-wasi/defined-symbols.txt \
        -Wl,--strip-all \
        -I $SCRIPT_DIR/app \
        -o app/vary_len.wasm $SCRIPT_DIR/app/safe_sync.c $SCRIPT_DIR/app/lz4.c
    
    compileaot $SCRIPT_DIR/app/vary_len
}

build_safe_async () {
    /opt/wasi-sdk/bin/clang $BM_CFLAGS \
        --target=wasm32-wasi \
        --sysroot=/opt/wasi-sdk/share/wasi-sysroot/ \
        -Wl,--allow-undefined-file=/opt/wasi-sdk/share/wasi-sysroot/share/wasm32-wasi/defined-symbols.txt \
        -Wl,--strip-all \
        -I $SCRIPT_DIR/app \
        -o app/vary_len.wasm $SCRIPT_DIR/app/safe_async.c $SCRIPT_DIR/app/lz4.c
    
    compileaot $SCRIPT_DIR/app/vary_len
}

build_sync () {
    /opt/wasi-sdk/bin/clang $BM_CFLAGS \
        --target=wasm32-wasi \
        --sysroot=/opt/wasi-sdk/share/wasi-sysroot/ \
        -Wl,--allow-undefined-file=/opt/wasi-sdk/share/wasi-sysroot/share/wasm32-wasi/defined-symbols.txt \
        -Wl,--strip-all \
        -I $SCRIPT_DIR/app \
        -o app/vary_len.wasm $SCRIPT_DIR/app/sync.c $SCRIPT_DIR/app/lz4.c
    
    compileaot $SCRIPT_DIR/app/vary_len
}

build_async () {
    /opt/wasi-sdk/bin/clang $BM_CFLAGS \
        --target=wasm32-wasi \
        --sysroot=/opt/wasi-sdk/share/wasi-sysroot/ \
        -Wl,--allow-undefined-file=/opt/wasi-sdk/share/wasi-sysroot/share/wasm32-wasi/defined-symbols.txt \
        -Wl,--strip-all \
        -I $SCRIPT_DIR/app \
        -o app/vary_len.wasm $SCRIPT_DIR/app/async.c $SCRIPT_DIR/app/lz4.c
    
    compileaot $SCRIPT_DIR/app/vary_len
}

SOMETHING=test.c

build_something (){
    echo "Compile file: $SOMETHING"
    /opt/wasi-sdk/bin/clang $BM_CFLAGS \
        --target=wasm32-wasi \
        --sysroot=/opt/wasi-sdk/share/wasi-sysroot/ \
        -Wl,--allow-undefined-file=/opt/wasi-sdk/share/wasi-sysroot/share/wasm32-wasi/defined-symbols.txt \
        -Wl,--strip-all \
        -I $SCRIPT_DIR/app \
        -o app/vary_len.wasm $SCRIPT_DIR/app/$SOMETHING $SCRIPT_DIR/app/lz4.c
    
    compileaot $SCRIPT_DIR/app/vary_len
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

