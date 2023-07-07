#!/bin/sh
## run all the tests in ./tests, after building them
## with the assembler.

line() {
	echo "-----------------------------------------------------------------"
}

cd tests

./build_tests.sh

cd ..

# build before testing, no sense in testing an
# out of date one
make

for file in $(find tests -name "*.bin"); do
	line
	echo -e "\e[32mRunning test $file...\e[0m"	# green text
	./nes "$file" && echo "Finished testing $file, exited with success." | lolcat -F 0.5 || echo -e "\e[31m./nes process exited with nonzero status code.\e[0m"
	line
done
