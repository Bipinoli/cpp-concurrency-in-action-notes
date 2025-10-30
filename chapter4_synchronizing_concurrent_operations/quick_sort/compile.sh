#!/bin/sh

mkdir -p build
clang++ -std=c++20 -O2 -fsanitize=thread -g main.cpp -o build/main
