#!/bin/sh
## build all the weird assets.

# build the bitmap font header directly.
./mk_bitmap_font.sh font.bmp 8 8 > my_font.data.c
