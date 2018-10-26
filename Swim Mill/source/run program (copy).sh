#!/bin/bash
#################################################
# Author: Chou Thao                             #
# Date:   10/26/2018                            #
# Description:                                  #
#    A small shell script to generate and run   #
# the swim_mill program.                        #
#################################################

# check to make sure all three c files are
# in the current directory
echo "Checking for files..."
if [ ! -f "fish.c" ] || [ ! -f "pellet.c" ] || [ ! -f "swim_mill.c" ] || [ ! -f "makefile" ]; then
    echo "One or more files are missing."
    echo "Please check to make sure fish.c, pellet.c, swim_mill.c, and makefile all exist in the current directory."
    exit 1
else
    echo "All files were found moving on..."
fi

# call make using the makefile in the current
# directory to generate the program files
echo "Creating program files..."
make

# check to ensure the program files were
# generated
echo "Checking to ensure program files were created..."
if [ ! -f fish ] || [ ! -f pellet ] || [ ! -f swim_mill ]; then
    echo "There was a program during the creation process of the program files."
    exit $exitCode
else
    echo "Program files were successfully created moving on..."
fi

# execute the program file
./swim_mill

exit 0
