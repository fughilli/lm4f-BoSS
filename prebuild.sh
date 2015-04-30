#!/bin/bash

# change pwd to script directory
cd $(dirname $0)

echo "Generating syscall_numbers.h"
python syscalllist_autogen.py && echo "Success" || echo "Fail"
echo "Generating Programs/osprogram.h"
python osprogram_gen.py && echo "Success" || echo "Fail"
echo "Checking system calls implementation..."
python check_syscalls.py