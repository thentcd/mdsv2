#!/bin/bash

if [[ "$OSTYPE" != "linux-gnu"* ]]; then
    echo "Error: This script is designed for Linux systems"
    exit 1
fi


if [ -f /etc/lsb-release ]; then
    source /etc/lsb-release
    echo "Detected: $DISTRIB_DESCRIPTION"
    
    if [[ "$DISTRIB_RELEASE" == "20.04" ]]; then
        echo "✓ Ubuntu/Lubuntu 20.04 detected - using Qt5"
    else
        echo "⚠ Warning: This script is optimized for Ubuntu 20.04"
        echo "Your version: $DISTRIB_RELEASE"
        read -p "Continue anyway? (y/n): " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
    fi
fi


command_exists() {
    command -v "$1" >/dev/null 2>&1
}

echo ""
echo "Installing dependencies for Qt5..."


sudo apt update


PACKAGES=(
    "build-essential"
    "cmake"
    "qt5-default"
    "qttools5-dev-tools"
    "qtbase5-dev"
    "libfftw3-dev"
    "pkg-config"
    "git"
)

echo "Installing packages: ${PACKAGES[*]}"

for package in "${PACKAGES[@]}"; do
    if ! dpkg -l | grep -q "^ii  $package "; then
        echo "Installing $package..."
        sudo apt install -y "$package"
        if [ $? -ne 0 ]; then
            echo "Failed to install $package"
            exit 1
        fi
    else
        echo "$package is already installed ✓"
    fi
done


echo ""
echo "Verifying Qt5 installation..."

if command_exists qmake; then
    echo "✓ qmake found: $(which qmake)"
    echo "✓ Qt version: $(qmake -query QT_VERSION)"
else
    echo "✗ qmake not found"
    exit 1
fi

if command_exists cmake; then
    echo "✓ CMake found: $(cmake --version | head -n1)"
else
    echo "✗ CMake not found"
    exit 1
fi


echo ""
echo "Setting up build environment..."
mkdir -p build
cd build


echo ""
echo "Configuring project with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local

if [ $? -ne 0 ]; then
    echo "✗ CMake configuration failed"
    echo ""
    echo "Troubleshooting tips:"
    echo "1. Make sure all Qt5 packages are installed"
    echo "2. Try: sudo apt install qtbase5-dev qt5-qmake"
    echo "3. Check if Qt5 is properly detected: qmake -query"
    exit 1
fi


echo ""
echo "Building project (this may take a few minutes)..."
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo "✗ Build failed"
    echo ""
    echo "Common issues:"
    echo "1. Missing Qt5 development headers"
    echo "2. Compiler errors - check if g++ supports C++17"
    echo "3. Missing dependencies"
    exit 1
fi


if [ ! -f "bin/CWTScalogramAnalyzer" ]; then
    echo "✗ Executable not found after build"
    exit 1
fi

echo ""
echo "🎉 Build Successful!"
echo ""
echo "📍 Executable location: $(pwd)/bin/CWTScalogramAnalyzer"
echo ""


echo "Testing executable..."
if ldd bin/CWTScalogramAnalyzer | grep -q "not found"; then
    echo "⚠ Warning: Missing shared libraries detected"
    echo "Libraries status:"
    ldd bin/CWTScalogramAnalyzer | grep "not found"
else
    echo "✓ All required libraries are available"
fi

echo ""
echo "📋 How to run:"
echo "  cd $(pwd)"
echo "  ./bin/CWTScalogramAnalyzer"
echo ""


echo "Generating example data..."
cd ..
if [ -f "generate_examples.py" ]; then
    if command_exists python3; then
        echo "Installing Python dependencies..."
        pip3 install numpy pandas scipy --user
        echo "Generating example signals..."
        python3 generate_examples.py
        echo "✓ Example data generated in examples/ directory"
    else
        echo "⚠ Python3 not found - skipping example generation"
        echo "Install python3 and run: python3 generate_examples.py"
    fi
fi

echo ""
echo "🚀 Quick Start Guide:"
echo "1. Run the application: cd build && ./bin/CWTScalogramAnalyzer"
echo "2. Load a CSV file using 'Load Signal File' button"
echo "3. Adjust parameters and click 'Perform CWT Analysis'"
echo "4. View results in the oscillogram and scalogram"
echo ""


read -p "Create desktop shortcut? (y/n): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    cd build
    EXEC_PATH=$(pwd)/bin/CWTScalogramAnalyzer
    ICON_PATH="applications-science"
    
    cat > ~/.local/share/applications/cwt-analyzer.desktop << EOF
[Desktop Entry]
Name=CWT Scalogram Analyzer
Comment=Continuous Wavelet Transform Analysis Tool for Multi-channel Signals
Exec=$EXEC_PATH
Icon=$ICON_PATH
Terminal=false
Type=Application
Categories=Science;Education;Engineering;
Keywords=signal;analysis;wavelet;CWT;oscillogram;scalogram;
StartupNotify=true
EOF
    chmod +x ~/.local/share/applications/cwt-analyzer.desktop
    echo "✓ Desktop entry created! Look for 'CWT Scalogram Analyzer' in your applications menu."
fi

echo ""
echo "✅ Installation completed successfully!"
echo ""
echo "📚 For help and documentation, see README.md"
echo "🐛 If you encounter issues, check the troubleshooting section"