#!/bin/bash

g++-12 tests.cpp \
    -lgtest \
    -fsanitize=address,undefined \
    -fno-sanitize-recover=all \
    -std=c++20 \
    -O2 \
    -Wall \
    -Werror \
    -Wsign-compare \
    -o test \
    && ./test
