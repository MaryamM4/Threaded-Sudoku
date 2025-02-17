#!/bin/bash

# Script to compile and run sudoku program

rm -f sudoku
gcc -g -fsanitize=address -Wall -Wextra -I../src ../src/sudoku.c -o sudoku -lpthread -lm


for file in tests/puzzle*.txt; do
    echo "==========================="
    echo "Puzzle File: $file"
    echo
    ./sudoku "$file"
    echo 
done
echo "All tests complete."

# to check for memory leaks, use
# valgrind ./sudoku puzzle9-good.txt

# to fix formating use
# clang-format -i main.c

# if clang-format does not work 
# use 'source scl_source enable llvm-toolset-7.0' and try again

# if using GitHub, you can run the program on GitHub servers and see
# the result. Repository > Actions > Run Workflow


