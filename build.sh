#!/bin/bash
if [[ "$(uname)" == "Linux" ]]; then
    echo "Running on Linux"
    libs=-lX11
    outputFile=schnitzel
else
    echo "Not running on Linux"
    libs=-luser32
    outputFile=schnitzel.exe
fi

warnings="-Wno-writable-strings -Wno-format-security"

clang src/main.cpp -g -o$outputFile $libs $warnings
