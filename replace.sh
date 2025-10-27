#!/bin/bash

version="3.36.1"
root="/home/pedro/Desktop/ns-allinone-3.36.1"

echo "Copying zash files..."
cp -avr "zash" "${root}/ns-${version}/src"

echo "Copying data files..."
cp -avr "data" "${root}/ns-${version}"

echo "Copying simulation zash files..."
cp -f "zash-simulator.cc" "${root}/ns-${version}/scratch/zash-simulator.cc"
cp -f "zash-simulator-conflict.cc" "${root}/ns-${version}/scratch/zash-simulator-conflict.cc"

echo "Copying run zash files..."
cp -f "run.sh" "${root}/ns-${version}/run.sh"
cp -f "run-conflict.sh" "${root}/ns-${version}/run-conflict.sh"

echo "Building ns-3..."
cd $root
./build.py