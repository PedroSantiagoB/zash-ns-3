#!/bin/bash

version="3.36.1"
root="/home/pedro/Desktop/ns-allinone-3.36.1"

echo "Copying zash files..."
cp -avr "zash" "${root}/ns-${version}/src"

echo "Copying data files..."
cp -avr "data" "${root}/ns-${version}"

echo "Copying simulation zash file..."
cp -f "zash-simulator.cc" "${root}/ns-${version}/scratch/zash-simulator.cc"

echo "Copying run zash file..."
cp -f "run.sh" "${root}/ns-${version}/run.sh"

echo "Building ns-3..."
cd $root
./build.py