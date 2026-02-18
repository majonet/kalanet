#!/bin/bash

echo "=========================================="
echo "KalaNet Build Script for Linux/macOS"
echo "=========================================="

# Check for qmake
if command -v qmake6 &> /dev/null; then
    QMAKE=qmake6
elif command -v qmake &> /dev/null; then
    QMAKE=qmake
else
    echo "Error: qmake not found! Please install Qt development tools."
    exit 1
fi

echo "Using: $QMAKE"

# Clean previous build
rm -rf build
mkdir -p build
cd build

# Run qmake
echo "Running qmake..."
$QMAKE ../KalaNet.pro
if [ $? -ne 0 ]; then
    echo "qmake failed!"
    exit 1
fi

# Build
echo "Building project..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo ""
echo "=========================================="
echo "Build successful!"
echo "Executable: build/KalaNet"
echo "=========================================="
echo ""
echo "Default login: admin / Admin123"
echo ""
