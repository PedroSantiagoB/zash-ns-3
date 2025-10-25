#!/bin/bash

echo "======================================"
echo "   ZASH Conflict Resolution Test"
echo "======================================"
echo ""
echo "Running conflict resolution scenarios..."
echo ""

cd /home/pedro/Desktop/ns-allinone-3.36.1/ns-3.36.1

# Run the conflict simulator
time ./build/scratch/ns3.36.1-zash-simulator-conflict-default

echo ""
echo "======================================"
echo "   Simulation Complete!"
echo "======================================"
echo ""
echo "Check the output above for test results."
echo "All 8 conflict scenarios have been executed."
echo ""

