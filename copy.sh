#!/bin/bash

# Define the target directory
target_directory="../pex/"

# List of files to copy
files_to_copy=("pe_exchange.c" "pe_exchange.h" "pe_routines.c" "pe_ll.c")

# Loop over the files and copy them to the target directory
for file in "${files_to_copy[@]}"
do
   cp -f "$file" "$target_directory"
done

echo "Files have been successfully copied to $target_directory"
