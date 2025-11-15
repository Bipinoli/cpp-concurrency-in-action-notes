#!/bin/bash

clang++ -std=c++20 -O3 -mavx2 -S -fverbose-asm -S matmul.cpp
