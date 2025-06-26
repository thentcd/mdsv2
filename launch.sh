#!/bin/bash

if [ ! -f "build/bin/mdsv2" ]; then
    echo "❌ Executable not found!"
    echo "Please build the project first:"
    echo "  cd .."
    echo "  ./build.sh"
    exit 1
fi


if [ ! -d "/usr/share/icons/hicolor" ]; then
    echo "Installing missing icon themes..."
    sudo apt install -y hicolor-icon-theme adwaita-icon-theme
fi


export QT_STYLE_OVERRIDE="Fusion"
export QT_QPA_PLATFORMTHEME="gtk2"

echo "Starting application..."


echo "Method 1: Standard launch"
./bin/CWTScalogramAnalyzer "$@" &
APP_PID=$!
sleep 2

if kill -0 $APP_PID 2>/dev/null; then
    echo "✅ Application started successfully!"
    wait $APP_PID
    exit 0
fi

echo "Method 2: Safe mode"
QT_STYLE_OVERRIDE=Fusion QT_QPA_PLATFORMTHEME="" ./build/bin/mdsv2 "$@"