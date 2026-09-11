#!/bin/bash

# Tarplex Scinfinity Engine Omega - Build Script for Linux/WSL

set -e

echo "================================================"
echo "Tarplex Scinfinity Engine Omega - Build Script"
echo "================================================"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check for required tools
echo -e "${YELLOW}Checking for required tools...${NC}"

if ! command -v cmake &> /dev/null; then
    echo -e "${RED}CMake not found. Please install CMake 3.16 or higher.${NC}"
    exit 1
fi
echo -e "${GREEN}✓ CMake found: $(cmake --version | head -n1)${NC}"

if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
    echo -e "${RED}C++ compiler not found. Please install GCC or Clang.${NC}"
    exit 1
fi
echo -e "${GREEN}✓ C++ Compiler found${NC}"

if ! command -v git &> /dev/null; then
    echo -e "${RED}Git not found. Please install Git.${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Git found${NC}"

echo ""

# Create build directory
echo -e "${YELLOW}Creating build directory...${NC}"
if [ ! -d "build" ]; then
    mkdir build
    echo -e "${GREEN}✓ Build directory created${NC}"
else
    echo -e "${GREEN}✓ Build directory already exists${NC}"
fi

cd build

# Run CMake
echo ""
echo -e "${YELLOW}Running CMake...${NC}"
cmake .. -DCMAKE_BUILD_TYPE=Release
echo -e "${GREEN}✓ CMake configuration complete${NC}"

# Build
echo ""
echo -e "${YELLOW}Building Tarplex Scinfinity Engine Omega...${NC}"
cmake --build . --config Release -j$(nproc)
echo -e "${GREEN}✓ Build complete${NC}"

# Optional: Run tests
echo ""
echo -e "${YELLOW}Running tests...${NC}"
if ctest --output-on-failure; then
    echo -e "${GREEN}✓ All tests passed${NC}"
else
    echo -e "${YELLOW}⚠ Some tests failed. Please review the output above.${NC}"
fi

echo ""
echo -e "${GREEN}================================================${NC}"
echo -e "${GREEN}Build completed successfully!${NC}"
echo -e "${GREEN}================================================${NC}"
echo ""
echo "To run the browser:"
echo "  ./bin/tarplex_browser"
echo ""
