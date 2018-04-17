#!/bin/sh

set -e

make cython
for file in examples/*.cy; do
	./cython $file;
done;
