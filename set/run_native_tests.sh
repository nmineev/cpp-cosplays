#!/bin/bash

g++ test_set.cpp set.h \
    -lgtest \
    -fsanitize=address,undefined \
    -fno-sanitize-recover=all \
    -std=c++17 \
    -O2 \
    -Wall \
    -Werror \
    -Wsign-compare \
    -o test \
    && ./test
