#!/bin/bash

# change pwd to script directory
cd $(dirname $0)

echo "Generating syscall_numbers.h"
python syscalllist_autogen.py && echo "Success" || echo "Fail"