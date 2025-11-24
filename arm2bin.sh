#!/bin/bash

INPUT_FILE="${1:-test.s}"
OUTPUT_BIN="${2:-test.bin}"

if [ ! -f "$INPUT_FILE" ]; then
    echo "Error: Input file '$INPUT_FILE' not found"
    exit 1
fi

aarch64-linux-gnu-gcc "$INPUT_FILE" -o "$OUTPUT_BIN" -L./lib -lsysy_aarch -static -g

echo "Successfully compiled $INPUT_FILE to $OUTPUT_BIN"
