#!/bin/bash

g++ solution.cpp \
    -fsanitize=address,undefined \
    -fno-sanitize-recover=all \
    -std=c++17 \
    -O2 \
    -Wall \
    -Werror \
    -Wsign-compare \
    -o solution \
    && ./solution
