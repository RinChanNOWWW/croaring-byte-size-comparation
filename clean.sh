#!/bin/bash

# Clean script to restore original v2 CMakeLists.txt

echo "Cleaning patched files..."

if [ -f "v1/CMakeLists.txt.original" ]; then
    echo "Restoring v1/CMakeLists.txt..."
    mv v1/CMakeLists.txt.original v1/CMakeLists.txt
    echo "Restored v1/CMakeLists.txt"
else
    echo "No backup found for v1/CMakeLists.txt"
fi

if [ -f "v2/CMakeLists.txt.original" ]; then
    echo "Restoring v2/CMakeLists.txt..."
    mv v2/CMakeLists.txt.original v2/CMakeLists.txt
    echo "Restored v2/CMakeLists.txt"
else
    echo "No backup found for v2/CMakeLists.txt"
fi

if [ -f "v2/src/CMakeLists.txt.original" ]; then
    echo "Restoring v2/src/CMakeLists.txt..."
    mv v2/src/CMakeLists.txt.original v2/src/CMakeLists.txt
    echo "Restored v2/src/CMakeLists.txt"
else
    echo "No backup found for v2/src/CMakeLists.txt"
fi

if [ -d "build" ]; then
    echo "Removing build directory..."
    rm -rf build
    echo "Build directory removed."
fi

echo "Clean completed!"
