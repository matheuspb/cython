#!/bin/sh

set -e

make cython
for file in examples/*.cy; do
	echo "compiling file $file"
	./cython $file;
done;
