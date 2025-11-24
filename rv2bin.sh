#!/bin/bash

INPUT_FILE="${1:-test.s}"
OUTPUT_BIN="${2:-test.bin}"
OBJ_FILE="${INPUT_FILE%.s}.o"

if [ -f "toolchains.conf" ]; then
    source toolchains.conf
fi

RISCV_GCC="${RISCV_GCC:-riscv64-unknown-elf-gcc}"
TEXT_ADDR="${TEXT_ADDR:-0x90000000}"

if [ ! -f "$INPUT_FILE" ]; then
    echo "Error: Input file '$INPUT_FILE' not found"
    exit 1
fi

"$RISCV_GCC" "$INPUT_FILE" -c -o "$OBJ_FILE" -w -g -Wa,--gdwarf-5
"$RISCV_GCC" "$OBJ_FILE" -o "$OUTPUT_BIN"\
    -L./lib -lsysy_riscv\
    -static -mcmodel=medany\
    -Wl,--no-relax,-Ttext="$TEXT_ADDR"

rm "$OBJ_FILE"

echo "Successfully compiled $INPUT_FILE to $OUTPUT_BIN"
