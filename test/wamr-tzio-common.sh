#!/bin/bash

# USER SETTINGS
BM_BOARD_HOSTNAME="192.168.13.2"
BM_BUILDER_HOSTNAME="127.0.0.1"
BM_BOARD_USER="root"
BM_BUILDER_PATH="/opt/watz-gpio/test"
BM_USE_PTA_IO_HELPER=1
BM_USE_ADAPTIVE_CHECKPOINT=0
BM_AOT_POLL_AT_BR=1
BM_AOT_POLL_AT_LOOP=0
BM_AOT_POLL=0
BM_INTERP_POLL_AT_BR=1
BM_INTERP_POLL_AT_LOOP=0
BM_INTERP_POLL=0

BM_ON_QEMU=1

if [ $BM_ON_QEMU ]; then
    BM_QEMU_SSH_PORT=22
    BM_QEMU_SSH_ARGS="-p ${BM_QEMU_SSH_PORT}"
    BM_QEMU_RSYNC_ARGS="-e 'ssh ${BM_QEMU_SSH_ARGS}'"
else
    BM_QEMU_SSH_ARGS=" "
    BM_QEMU_RSYNC_ARGS=" "
fi

# exit when any command fails
set -e

# Common settings
TA_LOG_LEVEL=4

# define common paths
BM_CFLAGS="-O3"
ROOT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
OPTEE_DIR=/opt/watz-gpio
OPTEE_OS_DIR=$OPTEE_DIR/optee_os-3.21.0
OPTEE_TOOLCHAINS_DIR=$OPTEE_DIR/toolchains
OPTEE_BR_OUT_DIR=$OPTEE_DIR/out-br
WATZ_RUNTIME_DIR=$ROOT_DIR/../runtime-modified
LOGS_DIR=$ROOT_DIR/logs

# Macros
announce () {
    echo "$(tput smso 2>/dev/null)>>> $1$(tput rmso 2>/dev/null)"
}

announcebuild () {
    announce "Building WaTZ benchmark: $1"
}

announcedeploy () {
    announce "Deploying WaTZ benchmark: $1"
}

announcerun () {
    announce "Running WaTZ benchmark: $1"
}

safesleep () {
    sleep 2
}

# Define the functions to build and deploy WaTZ (for TEE)
# Prototype: buildwatz <attester_data_size> <verifier_data_size> [make param1]
buildwatz () {
    announcebuild "runtime"
    mkdir -p $WATZ_RUNTIME_DIR/product-mini/platforms/linux-trustzone/build
    cd $WATZ_RUNTIME_DIR/product-mini/platforms/linux-trustzone/build
    cmake .. -DTZIO_INTERP_POLL=$BM_INTERP_POLL -DTZIO_INTERP_POLL_AT_LOOP=$BM_INTERP_POLL_AT_LOOP -DTZIO_INTERP_POLL_AT_BR=$BM_INTERP_POLL_AT_BR -DTZIO_INTERP_POLL=$BM_INTERP_POLL -DTZIO_AOT_POLL=$BM_AOT_POLL -DTZIO_AOT_POLL_AT_LOOP=$BM_AOT_POLL_AT_LOOP -DTZIO_AOT_POLL_AT_BR=$BM_AOT_POLL_AT_BR -DUSE_PTA_IO_HELPER=$BM_USE_PTA_IO_HELPER -DUSE_ADAPTIVE_CHECKPOINT=$BM_USE_ADAPTIVE_CHECKPOINT
    make

    announcebuild "wamr_tzio "
    cd $WATZ_RUNTIME_DIR/product-mini/platforms/linux-trustzone/wamr_tzio
    make clean
    make CROSS_COMPILE=$OPTEE_TOOLCHAINS_DIR/aarch64/bin/aarch64-linux-gnu- \
        TEEC_EXPORT=$OPTEE_BR_OUT_DIR/host/aarch64-buildroot-linux-gnu/sysroot/usr \
        TA_DEV_KIT_DIR=$OPTEE_OS_DIR/out/arm/export-ta_arm64 TA_DATA_SIZE=$1 CFG_TEE_TA_LOG_LEVEL=$TA_LOG_LEVEL $3
}

deploywatz () {
    announcedeploy "wamr_tzio"
    mkdir -p $ROOT_DIR/build
    cd $ROOT_DIR/build
    rsync --progress -r $BM_BUILDER_HOSTNAME:$BM_BUILDER_PATH/../runtime-modified/product-mini/platforms/linux-trustzone/wamr_tzio/out/\* out/
    sshpass -p "$BM_BOARD_USER" rsync -e 'ssh -p 22 -o StrictHostKeyChecking=no' --progress out/ca/wamr_tzio $BM_BOARD_USER@$BM_BOARD_HOSTNAME:/usr/bin
    sshpass -p "$BM_BOARD_USER" rsync -e 'ssh -p 22 -o StrictHostKeyChecking=no' --progress out/ta/bc20728a-6a28-49d8-98d8-f22e7535f138.ta $BM_BOARD_USER@$BM_BOARD_HOSTNAME:/lib/optee_armtz
    sshpass -p "$BM_BOARD_USER" ssh $BM_BOARD_USER@$BM_BOARD_HOSTNAME $BM_QEMU_SSH_ARGS 'chmod 666 /lib/optee_armtz/bc20728a-6a28-49d8-98d8-f22e7535f138.ta'
}

# Define the functions to build and deploy WAMR (for REE)
buildwamr () {
    announcebuild "WAMR runtime"
    mkdir -p $WATZ_RUNTIME_DIR/product-mini/platforms/linux/build
    cd $WATZ_RUNTIME_DIR/product-mini/platforms/linux/build
    cp ../CMakeLists-aarch64.txt ../CMakeLists.txt
    cmake ..
    make clean
    make
    rm ../CMakeLists.txt
}

deploywamr () {
    announcedeploy "WAMR runtime"
    mkdir -p $WATZ_RUNTIME_DIR/product-mini/platforms/linux/build
    cd $WATZ_RUNTIME_DIR/product-mini/platforms/linux/build
    rsync --progress -r $BM_BUILDER_HOSTNAME:$BM_BUILDER_PATH/../runtime-modified/product-mini/platforms/linux/build/iwasm .
    sshpass -p "$BM_BOARD_USER" rsync -e 'ssh -p 22 -o StrictHostKeyChecking=no' --progress iwasm $BM_BOARD_USER@$BM_BOARD_HOSTNAME:/usr/bin
}

buildaotcompiler () {
    cd $WATZ_RUNTIME_DIR/wamr-compiler
    
    if [ ! -d "../core/deps/llvm" ]; then
        announce "Building LLVM"
        ./build_llvm.sh
    fi

    announce "Building AoT compiler"
    mkdir -p build
    cd build
    cmake .. -DTZIO_INTERP_POLL=$BM_INTERP_POLL -DTZIO_INTERP_POLL_AT_LOOP=$BM_INTERP_POLL_AT_LOOP -DTZIO_INTERP_POLL_AT_BR=$BM_INTERP_POLL_AT_BR -DTZIO_INTERP_POLL=$BM_INTERP_POLL -DTZIO_AOT_POLL=$BM_AOT_POLL -DTZIO_AOT_POLL_AT_LOOP=$BM_AOT_POLL_AT_LOOP -DTZIO_AOT_POLL_AT_BR=$BM_AOT_POLL_AT_BR -DUSE_PTA_IO_HELPER=$BM_USE_PTA_IO_HELPER -DUSE_ADAPTIVE_CHECKPOINT=$BM_USE_ADAPTIVE_CHECKPOINT
    make
}

buildaotcompiler_cross () {
    cd $WATZ_RUNTIME_DIR/wamr-compiler
    
    if [ ! -d "../core/deps/llvm" ]; then
        announce "Building LLVM"
        ./build_llvm.sh
    fi

    announce "Building AoT compiler cross"
    mkdir -p build
    cd build
    cmake .. \
        -DCMAKE_SYSROOT=$OPTEE_BR_OUT_DIR/host/aarch64-buildroot-linux-gnu/sysroot  \
        -DWAMR_BUILD_TARGET=AARCH64 \
        -DTA_DEV_KIT_DIR=$OPTEE_OS_DIR/out/arm/export-ta_arm64
    make
}

# check ir add --format=llvmir-unopt
compileaot () {
    # Configure the bounds checks similarly to SGX.
    # Use the large size; small and tiny cannot be properly compiled.
    $WATZ_RUNTIME_DIR/wamr-compiler/build/wamrc \
        -v=3    \
        --target=aarch64 \
        --bounds-checks=0 \
        --size-level=0 \
        -o $1.aot \
        $1.wasm
}

restart_tee_supplicant() {
    sshpass -p $BM_BOARD_USER ssh $BM_BOARD_USER@$BM_BOARD_HOSTNAME $BM_QEMU_SSH_ARGS "killall tee-supplicant"
    safesleep
    sshpass -p $BM_BOARD_USER ssh $BM_BOARD_USER@$BM_BOARD_HOSTNAME $BM_QEMU_SSH_ARGS "/etc/init.d/S30optee start"
    safesleep
}

# Set up the common environment
mkdir -p $LOGS_DIR
