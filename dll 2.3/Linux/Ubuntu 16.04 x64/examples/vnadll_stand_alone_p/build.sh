#!/bin/bash

set -e

if [ ! -f libvnadll.so ]; then
	cp ../../build/dlls/vnadll/libvnadll.so libvnadll.so
fi

echo Running Python3 Unit-Tests
python3 vna_test.py
sleep 3   # Let the port be free-ed again
echo Running Python3 Demo
python3 simple_demo.py
sleep 3   # Let the port be free-ed again

echo Running Python2 Unit-Tests
python vna_test.py
sleep 3   # Let the port be free-ed again
echo Running Python2 Demo
python simple_demo.py
