#!/bin/bash -e 

if [ "$#" -lt 1 ];
then
    echo "Usage: ./test.sh <name>"
    echo "Usage:        E.g., ./test.sh read"
    exit 0
fi

if [[ $1 == *"-variable" ]]; then
    if [ "$#" -lt 4 ];
    then
        echo "Usage: ./test.sh $1 <blocksize> <iterations> <offset>"
        echo "Usage:        E.g., ./test.sh test-read-variable 512 1000 0"
        exit 0
    fi
fi

# Check which /dev entry contains the USB device
# NOTE: make sure your USB disk is exactly 1G or this command will not work
dpath=/dev/`lsblk | grep 1G | cut -d ' ' -f 1`
if [ "$dpath" == "/dev/" ];
then
        dpath=/dev/`lsblk | grep 1024M | cut -d ' ' -f 1`
fi
echo "Device ==> $dpath"

# Build the kernel module and insert it with the correct device name
pushd kmodule
    make
    make remove
	sudo insmod kmod.ko device=$dpath 
popd

# Run a testcase using provided arguments
#   Usage:      ./test.sh <test> <operationsize> <count> <offset>
#   Example:    ./test.sh read-variable 512 1 0
#   Example:    ./test.sh read
pushd testcases
    rm -rf *.txt
    make clean
    make test-$1
    echo ""
    echo ""
    echo "----------------------------TESTCASE---------------------------"
    sudo ./test-$1 $dpath $2 $3 $4
popd
