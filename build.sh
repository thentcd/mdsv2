#!/bin/bash


echo "=== CWT Scalogram Analyzer Build Script ==="
echo ""


if [[ "$OSTYPE" != "linux-gnu"* ]]; then
    echo "Error: This script is designed for Linux systems"
    exit 1
fi


command_exists() {
    command -v "$1" >/dev/null 2>&1
}


echo "Checking and installing dependencies..."


sudo apt update


PACKAGES=(
    "build-essential"
    "cmake"
    "qt6-base-dev"
    "qt6-base-dev-tools"
    "libfftw3-dev"
    "pkg-config"
)

for package in "${PACKAGES[@]}"; do
    if ! dpkg -l | grep -q "^ii  $package "; then
        echo "Installing $package..."
        sudo apt install -y "$package"
    else
        echo "$package is already installed"
    fi
done


if ! command_exists qmake6; then
    echo "Error: Qt6 qmake not found. Please install Qt6 development tools."
    exit 1
fi

echo ""
echo "Qt6 version: $(qmake6 --version)"


echo ""
echo "Creating build directory..."
mkdir -p build
cd build


echo ""
echo "Configuring project with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

if [ $? -ne 0 ]; then
    echo "Error: CMake configuration failed"
    exit 1
fi


echo ""
echo "Building project..."
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo "Error: Build failed"
    exit 1
fi


if [ ! -f "bin/CWTScalogramAnalyzer" ]; then
    echo "Error: Executable not found"
    exit 1
fi

echo ""
echo "=== Build Successful! ==="
echo ""
echo "Executable location: $(pwd)/bin/CWTScalogramAnalyzer"
echo ""
echo "To run the application:"
echo "  cd $(pwd)"
echo "  ./bin/CWTScalogramAnalyzer"
echo ""
echo "Example CSV files are available in the examples/ directory"
echo ""


read -p "Create desktop entry? (y/n): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    cat > ~/.local/share/applications/cwt-analyzer.desktop << EOF
[Desktop Entry]
Name=CWT Scalogram Analyzer
Comment=Continuous Wavelet Transform Analysis Tool
Exec=$(pwd)/bin/CWTScalogramAnalyzer
Icon=applications-science
Terminal=false
Type=Application
Categories=Science;Education;
EOF
    echo "Desktop entry created!"
fi

echo ""
echo "Build script completed successfully!"