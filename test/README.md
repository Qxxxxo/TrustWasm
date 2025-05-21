Tests for TrustWasm

----

```
 ___________               _______________ 
|           |     ssh     |               |1.image flash in
|  Rpi3b+   |D===========C| Build Machine |SD|
|___________| <-Deploy--  |_______________|
|SD| |serial|                      ||
 TT    TTTT    3.build test        ||
 2.    ||||    4.deploy runtime    ||
 plugin||||      & test wasm aot   ||
 reset ||||    5.run test          ||
 ______IIII                        ||                     
|          |        ssh            ||
| Develop  |D======================]]
| Machine  |
|__________|        
```

For example, run commands below:
```
cd ./test/rpi3-gpio/
./build.sh
./deploy.sh
./run.sh
```
----
## Benchmarks
For example (rpi3b+ ipv4 is configured as 192.168.13.2, while Build Machine ipv4 is configured as 192.168.13.1 )

```
cd ./test/benchmarks/trusted_storage
./build.sh sync.c
./deploy aot
./run aot

```
---
## Updates
We add an experiment for power and sample rate under `trusted_warning_power`.


