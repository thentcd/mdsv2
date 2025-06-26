#!/bin/bash



if [ ! -f "CMakeLists.txt" ]; then
    echo "❌ CMakeLists.txt not found!"
    echo "Run this script from the project root directory."
    exit 1
fi


echo "Installing dependencies..."
sudo apt update
sudo apt install -y build-essential cmake qt5-default qttools5-dev-tools qtbase5-dev libfftw3-dev pkg-config


echo ""
echo "Setting up build directory..."
rm -rf build
mkdir build
cd build


echo ""
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

if [ $? -ne 0 ]; then
    echo "❌ CMake configuration failed!"
    exit 1
fi


echo ""
echo "Building project..."
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo "❌ Build failed!"
    exit 1
fi


if [ -f "build/bin/mdsv2" ]; then
    echo ""
    echo "✅ Build successful!"
    echo "Executable created: $(pwd)/build/bin/mdsv2"
    echo ""
    echo "To run the application:"
    echo "  cd build"
    echo "  ./launch.sh"
else
    echo "❌ Executable not found after build!"
    exit 1
fi