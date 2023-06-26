#!/bin/bash

outputFile=schnitzel
src_base="src/"
projectFiles=("main.cpp")

if [[ "$(uname)" == "Windows" ]]; then
    echo "Running on Windows"
    libs=(-luser32)
    projectFiles+=("windows_platform.cpp")
elif [[ "$(uname)" == "Linux" ]]; then
    echo "Running on Linux"
    libs=(-lX11)
    projectFiles+=("linux_platform.cpp")
elif [[ "$(uname)" == "Darwin" ]]; then
    echo "Running on Mac"
    libs=(-framework Cocoa)
    projectFiles+=("mac_platform.mm")
    sdkpath=$(xcode-select --print-path)/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk
    cflags="-isysroot ${sdkpath} -I${sdkpath}/System/Library/Frameworks/Cocoa.framework/Headers"
else
    echo "Unsupported platform"
    exit 1
fi

warnings=-Wno-writable-strings
sourceFiles=("${projectFiles[@]/#/${src_base}}")

clang $cflags -g -o $outputFile "${sourceFiles[@]}" "${libs[@]}" $warnings
