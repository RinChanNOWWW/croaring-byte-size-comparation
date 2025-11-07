#!/bin/bash

# Roaring Bitmap v1 vs v2 Comparison Build Script

set -e  # Exit on error

echo "========================================"
echo "Roaring Bitmap v1 vs v2 Comparison"
echo "========================================"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if build directory exists
if [ -d "build" ]; then
    echo -e "${YELLOW}Build directory exists. Cleaning...${NC}"
    rm -rf build
fi

# Create build directory
echo -e "${GREEN}Creating build directory...${NC}"
mkdir build
cd build

# Configure
echo -e "${GREEN}Configuring project...${NC}"
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
echo -e "${GREEN}Building project...${NC}"
cmake --build . -j$(nproc)

# Check if build succeeded
if [ -f "main" ]; then
    echo ""
    echo -e "${GREEN}========================================"
    echo "Build successful!"
    echo "========================================${NC}"
    echo ""
    echo "To run the comparison:"
    echo "  cd build"
    echo "  ./main"
    echo ""
    echo "Or run directly:"
    echo "  ./build/main"
    echo ""
    echo -e "${NC}"  # Ensure color is reset at the end
else
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi
