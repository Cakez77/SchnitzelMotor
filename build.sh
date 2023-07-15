#!/bin/bash

warnings="-Wno-writable-strings -Wno-format-security -Wno-c++11-extensions -Wno-deprecated-declarations"
timestamp=$(date +%s)

if [[ "$(uname)" == "Linux" ]]; then
    echo "Running on Linux"
    libs="-lX11 -lGL"
    includes="-Ithird_party"
    outputFile=schnitzel
    queryProcesses=$(pgrep $outputFile)
    # fPIC position independent code
    clang++ -g "src/game.cpp" -shared -fPIC -o game_$timestamp.so $warnings
    mv game_$timestamp.so game.so

elif [[ "$(uname)" == "Darwin" ]]; then
    echo "Running on Mac"
    libs="-framework Cocoa"
    sdkpath=$(xcode-select --print-path)/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk
    includes="-Ithird_party -isysroot ${sdkpath} -I${sdkpath}/System/Library/Frameworks/Cocoa.framework/Headers"
    objc_dep="src/mac_platform.m"
    outputFile=schnitzel
    # clean up old object files
    rm -f src/*.o
else
    echo "Not running on Linux"
    libs="-luser32 -lgdi32 -lopengl32 -lole32"
    includes="-Ithird_party"
    outputFile=schnitzel.exe
    queryProcesses=$(tasklist | grep $outputFile)

    rm -f game_* # Remove old game_* files
    clang++ -g "src/game.cpp" -shared -o game_$timestamp.dll $warnings
    mv game_$timestamp.dll game.dll
fi

processRunning=$queryProcesses

if [ -z "$processRunning" ]; then
    echo "Engine not running, building main..."
    clang++ $includes -g "src/main.cpp" $objc_dep -o $outputFile $libs $warnings
else
    echo "Engine running, not building!"
fi

