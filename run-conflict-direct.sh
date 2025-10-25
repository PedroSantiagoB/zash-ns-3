#!/bin/bash

# Este script roda a simulação de conflitos usando o comando ns3
# Ele pode ser executado tanto da raiz do zash-ns-3 quanto do ns-3.36.1

echo "======================================"
echo "   ZASH Conflict Resolution Test"
echo "======================================"
echo ""

# Detectar se estamos na raiz do ZASH ou do NS-3
if [ -f "ns3" ]; then
    # Estamos no NS-3
    NS3_DIR="."
elif [ -d "/home/pedro/Desktop/ns-allinone-3.36.1/ns-3.36.1" ]; then
    # Estamos no zash-ns-3, vamos para o NS-3
    NS3_DIR="/home/pedro/Desktop/ns-allinone-3.36.1/ns-3.36.1"
else
    echo "Error: NS-3 directory not found!"
    exit 1
fi

cd "$NS3_DIR"

echo "Running conflict resolution scenarios..."
echo ""

# Run the conflict simulator
time ./ns3 run scratch/zash-simulator-conflict

echo ""
echo "======================================"
echo "   Simulation Complete!"
echo "======================================"
echo ""
echo "Check the output above for test results."
echo "All 8 conflict scenarios have been executed."
echo ""

