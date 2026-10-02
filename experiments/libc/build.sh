#!/bin/sh
set -eu
origin=$1
libc=$2
compiler=$3
mkdir -p /out
"$compiler" -g0 -std=c17 -Wall -Wextra -Wpedantic -c /work/main.c -o "/work/$libc.o"
"$compiler" -Wl,-S "/work/$libc.o" -o "/out/$origin-$libc-dynamic"
"$compiler" -Wl,-S -static "/work/$libc.o" -o "/out/$origin-$libc-static"
