#!/bin/bash

# Remove all the .out files in the path passed to it
clear_all_out_files() {
    find $1 -name "*.out" -type f -delete
}
# Compile all the .c files in the path passed to it
compile_all_c_files() {
    # Loop through all the .c files in the path passed to it
    for file in $1/*.c
    do
        # Get file name without extension
        filename=$(basename "$file" .c)
        
        # grep the file and check if it contains the regular expression "main\b"
        if grep -i -q "\b$2\b" $file
        then
            # If it does, compile the file
            gcc -w -o "$1/$filename.out" $file
        fi
    done
}

# If the number of arguments is less than 2
if [ $# -lt 2 ]
then
    echo "Not enough parameters"
    exit 1
fi

clear_all_out_files $1
compile_all_c_files $1 $2

# If the -r flag is passed
if [ $# -eq 3 ]
then
    if [ $3 = "-r" ]
    then
        # Call the script recursively and pass it all the folders in the path
        for file in $1/*
        do
            # If file is a directory
            if [ -d $file ]
            then
                "$0" "$file" "$2" "-r"
            fi
        done
    fi
fi