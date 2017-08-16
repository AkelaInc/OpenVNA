#!/bin/bash

set -e

if [ ! -f libvnadll.so ]; then
	cp ../../build/dlls/vnadll/libvnadll.so libvnadll.so
fi

g++ -o vnadll_test.o -c -fPIC "-D UNICODE" "-D NDEBUG" "-D _UNICODE" "-D _CONSOLE" "-D LINUX" "-D GCC" -fstack-protector -std=c++11 -D_FILE_OFFSET_BITS=64 -Wno-unused-variable -Wno-write-strings -Wno-redundant-decls -fno-omit-frame-pointer -g -Og -fstack-check -I. vnadll_test.cpp
g++ -o vnadll_test.bin -std=c++11 -pthread -g vnadll_test.o libvnadll.so

LD_LIBRARY_PATH=. ./vnadll_test.bin