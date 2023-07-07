#!/bin/sh
## build all the test .sasm (simple assembler) files with
## the simple_assembler python tool, locally installed.

for file in $(find -name "*.sasm"); do
	simple_assembler "$file"
done
