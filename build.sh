#!/bin/bash

libs=-luser32
warnings=-Wno-writable-strings

clang src/main.cpp -oschnitzel.exe $libs $warnings
