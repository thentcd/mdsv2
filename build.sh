#!/bin/bash





SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXECUTABLE="$SCRIPT_DIR/bin/CWTScalogramAnalyzer"


if [ ! -f "$EXECUTABLE" ]; then
    echo "Error: Executable not found at $EXECUTABLE"
    echo "Please build the project first:"
    echo "  ./build_qt5.sh"
    exit 1
fi


install_themes() {
    echo "Installing missing icon themes..."
    sudo apt update
    sudo apt install -y hicolor-icon-theme adwaita-icon-theme elementary-icon-theme
    
    
    sudo gtk-update-icon-cache /usr/share/icons/* 2>/dev/null || true
}


if [ ! -d "/usr/share/icons/hicolor" ] && [ ! -d "/usr/share/icons/Adwaita" ]; then
    echo "Warning: Basic icon themes not found"
    read -p "Install icon themes now? (y/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        install_themes
    fi
fi


export QT_QPA_PLATFORMTHEME="gtk2"
export QT_STYLE_OVERRIDE="Fusion"
export QT_AUTO_SCREEN_SCALE_FACTOR=0
export QT_SCALE_FACTOR=1

s
if [ "$XDG_CURRENT_DESKTOP" = "LXQt" ] || [ "$XDG_CURRENT_DESKTOP" = "LXDE" ]; then
    echo "Detected Lubuntu/LXDE environment"
    export QT_QPA_PLATFORMTHEME="lxqt"
    export QT_STYLE_OVERRIDE=""
fi


if [ -d "/usr/share/icons/hicolor" ]; then
    export QT_QPA_ICON_THEME="hicolor"
elif [ -d "/usr/share/icons/Adwaita" ]; then
    export QT_QPA_ICON_THEME="Adwaita"
fi


echo "Environment:"
echo "  Desktop: $XDG_CURRENT_DESKTOP"
echo "  Qt Platform Theme: $QT_QPA_PLATFORMTHEME"
echo "  Qt Style: $QT_STYLE_OVERRIDE"
echo "  Icon Theme: $QT_QPA_ICON_THEME"


try_launch() {
    local method="$1"
    local cmd="$2"
    
    echo ""
    echo "Trying launch method: $method"
    echo "Command: $cmd"
    
    
    cat > /tmp/cwt_launch.sh << EOF
#!/bin/bash
cd "$SCRIPT_DIR"
$cmd
echo "Exit code: \$?"
EOF
    chmod +x /tmp/cwt_launch.sh
    
    
    timeout 10s /tmp/cwt_launch.sh
    local exit_code=$?
    
    rm -f /tmp/cwt_launch.sh
    
    if [ $exit_code -eq 0 ]; then
        echo "✓ Launch method '$method' worked!"
        return 0
    elif [ $exit_code -eq 124 ]; then
        echo "✓ Application started (timeout reached - this is good)"
        return 0
    else
        echo "✗ Launch method '$method' failed (exit code: $exit_code)"
        return 1
    fi
}


echo ""
echo "Attempting to launch application..."


if try_launch "Standard" "$EXECUTABLE"; then
    exec "$EXECUTABLE" "$@"
fi


if try_launch "GTK2 Theme" "QT_QPA_PLATFORMTHEME=gtk2 $EXECUTABLE"; then
    exec env QT_QPA_PLATFORMTHEME=gtk2 "$EXECUTABLE" "$@"
fi


if try_launch "Fusion Style" "QT_STYLE_OVERRIDE=Fusion $EXECUTABLE"; then
    exec env QT_STYLE_OVERRIDE=Fusion "$EXECUTABLE" "$@"
fi


if try_launch "Minimal Env" "QT_QPA_PLATFORMTHEME= QT_STYLE_OVERRIDE=Fusion $EXECUTABLE"; then
    exec env QT_QPA_PLATFORMTHEME= QT_STYLE_OVERRIDE=Fusion "$EXECUTABLE" "$@"
fi


if try_launch "X11 Platform" "QT_QPA_PLATFORM=xcb $EXECUTABLE"; then
    exec env QT_QPA_PLATFORM=xcb "$EXECUTABLE" "$@"
fi


echo ""
echo "All standard methods failed. Trying debug mode..."
echo "This will show detailed error information:"
echo ""


export QT_LOGGING_RULES="*.debug=true"
export QT_DEBUG_PLUGINS=1


cd "$SCRIPT_DIR"
gdb --batch --ex run --ex bt --args "$EXECUTABLE" 2>&1 | head -50

echo ""
echo "=== Troubleshooting ==="
echo "If the application still crashes, try:"
echo ""
echo "1. Install additional packages:"
echo "   sudo apt install qt5-gtk-platformtheme"
echo "   sudo apt install libqt5gui5"
echo ""
echo "2. Update your system:"
echo "   sudo apt update && sudo apt upgrade"
echo ""
echo "3. Check system logs:"
echo "   dmesg | tail"
echo "   journalctl --user -n 20"
echo ""
echo "4. Run manually with different options:"
echo "   cd $SCRIPT_DIR"
echo "   QT_QPA_PLATFORMTHEME=gtk2 ./bin/CWTScalogramAnalyzer"
echo "   QT_STYLE_OVERRIDE=Windows ./bin/CWTScalogramAnalyzer"
echo ""
echo "5. Check library dependencies:"
echo "   ldd $EXECUTABLE"
echo ""

exit 1