#!/bin/sh

mkdir -p build
clang++ -std=c++20 -mavx2 -O3 -fsanitize=address matmul.cpp -o build/main
