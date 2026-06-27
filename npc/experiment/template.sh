#!/bin/bash

if [ $# -eq 0 ]; then
    echo "usage: $0 <name>"
    exit 1
elif [ -d $1 ]; then
    echo "$1 exists"
else 
    mkdir -p $1/constr $1/csrc $1/vsrc
    touch $1/constr/top.nxdc $1/csrc/main.cpp $1/vsrc/top.sv
    echo -e "TOPNAME=top\n\ninclude ../template.mk" >> $1/Makefile
    echo "finished"
fi