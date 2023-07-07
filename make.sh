#!/bin/sh

# build the assets
if [ ${1:-"n"} == "assets" ]; then
	cd assets 
	./build_assets.sh 
	cd ..
fi

# build the program
gcc -o nes *.c -lGL -lglfw -lGLEW -g
