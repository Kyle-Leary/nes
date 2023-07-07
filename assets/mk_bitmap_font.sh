#!/bin/sh
## creation of a bitmap font from a .bmp image file,
## a width, and a height.
## uses xxd from vim, which has a nice binary->c array formatting
## feature.
##
## pipe the output of this command into a file. this should be header-ready, without any extra editing. 
## use the output file as a c file, not as a .h.

# Check arguments
if [ "$#" -ne 3 ]; then
    echo "Usage: $0 image.bmp char_width char_height"
    exit 1
fi

image=$1
char_width=$2
char_height=$3

num_pixels=$(($char_height * $char_width))

characters=128

clean_image_path=$(sanitize $image)

# this is just one column of 128 characters, going all the way down.
for (( i=0; i<$characters; i++ )); do
    # 0x28 is the size of the bitmap header.
    xxd -s $((0x28 + $num_pixels * $i)) -l $num_pixels -i "$image" | sed "s/$(realname $image)/bitmap_$i/g" # change the generic array name
done

# then, create the main bitmap array, with all the xxd arrays as members of it.
echo "unsigned char *bitmap_font[128] = {"
for (( i=0; i<$characters; i++ )); do
    echo "  bitmap_${i}_bmp,"
done
echo "};"
