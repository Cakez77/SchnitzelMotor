#!/bin/bash

if [[ "$(uname)" == "Linux" ]]; then
    echo "__ linux __"
    libs=-lX11
    outputFile=schnitzel
elif [[ "$(uname)" == "Darwin" ]]; then
    echo "__ mac __"
    libs="-framework Cocoa"
    sdkpath=$(xcode-select --print-path)/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk
    cflags="-isysroot ${sdkpath} -I${sdkpath}/System/Library/Frameworks/Cocoa.framework/Headers"
    objc_dep="src/mac_platform.m"
    outputFile=schnitzel
    # clean up old object files
    rm -f src/*.o
else
    echo "__ windows __"
    libs="-luser32 -lgdi32 -lopengl32"
    outputFile="schnitzel.exe"
fi

warnings="-Wno-writable-strings -Wno-format-security -Wno-c++11-extensions -Wno-deprecated-declarations"
clang $cflags -g "src/main.cpp" $objc_dep -o $outputFile $libs $warnings
