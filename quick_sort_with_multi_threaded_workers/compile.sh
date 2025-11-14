#!/bin/sh

mkdir -p build
clang++ -std=c++20 -Wall -O0 -fsanitize=thread -g main.cpp -o build/main
