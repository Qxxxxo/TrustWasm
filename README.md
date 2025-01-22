# TrustWasm
### Setup

To build the project, follow the [OPTEE build docs](https://optee.readthedocs.io/en/latest/building/gits/build.html#build).
- ./build: contains optee build makefiles
- ./buildroot: https://github.com/buildroot/buildroot 
- ./firmware: https://github.com/raspberrypi/firmware
- ./linux: optee forked linux source code, see also https://github.com/linaro-swg/linux/
- ./uboot: https://github.com/u-boot/u-boot/
- ./trusted-firmware-a: https://github.com/ARM-software/arm-trusted-firmware
- ./toolchains: toolchains for cross compilation

    For example, x86_64->aarch64 download [here](https://developer.arm.com/downloads/-/gnu-a/9-2-2019-12#:~:text=none%2Delf.tar.xz.asc-,AArch64%20GNU/Linux%20target%20(aarch64%2Dnone%2Dlinux%2Dgnu),-gcc%2Darm%2D9.2%2D2019.12%2Dx86_64).
    x86_64->aarch32 download [here](https://developer.arm.com/downloads/-/gnu-a/9-2-2019-12#:~:text=none%2Deabi.tar.xz.asc-,AArch32%20target%20with%20hard%20float%20(arm%2Dnone%2Dlinux%2Dgnueabihf),-gcc%2Darm%2D9.2%2D2019.12%2Dx86_64).
    ```
    mkdir toolchains
    ```
    ```
    ./toolchains/
    ├── aarch32/  <---gcc-arm-9.2-2019.12-x86_64-arm-none-linux-gnueabihf.tar.xz
    ├── aarch64/   <---gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu.tar.xz

    ```
Build image.

```
cd build
make
```
Then burn the image to the sd card, just follow instructions in ./build/rpi3.mk.

The SD card with the flashed image can be inserted into the board to power it on and run.

To run tests, see ./test/README.md.


### Dirs Description
- ./optee_client-3.21.0: see also https://github.com/OP-TEE/optee_client
- ./optee_examples: we added some native benchmark apps here.
- ./optee_os-3.21.0: we added gpio, i2c and spi drivers for rpi3b, some sensor drivers and io helper PTA.
- ./optee_test-3.21.0: we added out native lib in ./optee_test-3.21.0/ta/native_tzio
- ./runtime-modified: we modified WAMR to support async io (future and callback).
- ./test: contains our example wasm applications for IO functions and benchmarks, check ./test/README.md for more details.
