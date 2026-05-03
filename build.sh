#!/usr/bin/env bash

set -e

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
OUTPUT_DIR="$ROOT_DIR/output"

WIN32_DIR="$OUTPUT_DIR/windows/32-bit"
WIN64_DIR="$OUTPUT_DIR/windows/64-bit"
LINUX32_DIR="$OUTPUT_DIR/linux/32-bit"
LINUX64_DIR="$OUTPUT_DIR/linux/64-bit"

CXX_STANDARD="-std=c++23"
COMMON_FLAGS="-O2 -static -static-libgcc -static-libstdc++"

mkdir -p "$WIN32_DIR" "$WIN64_DIR" "$LINUX32_DIR" "$LINUX64_DIR"

echo "Cleaning old output files..."
rm -f "$WIN32_DIR"/* "$WIN64_DIR"/* "$LINUX32_DIR"/* "$LINUX64_DIR"/*

build_windows_32() {
  local source_file="$1"
  local output_name="$2"

  echo "Building Windows 32-bit: $output_name.exe"

  i686-w64-mingw32-g++ \
    $CXX_STANDARD \
    $COMMON_FLAGS \
    "$ROOT_DIR/$source_file" \
    -o "$WIN32_DIR/$output_name.exe"
}

build_windows_64() {
  local source_file="$1"
  local output_name="$2"

  echo "Building Windows 64-bit: $output_name.exe"

  x86_64-w64-mingw32-g++ \
    $CXX_STANDARD \
    $COMMON_FLAGS \
    "$ROOT_DIR/$source_file" \
    -o "$WIN64_DIR/$output_name.exe"
}

build_linux_32() {
  local source_file="$1"
  local output_name="$2"

  echo "Building Linux 32-bit: $output_name"

  g++ -m32 \
    $CXX_STANDARD \
    $COMMON_FLAGS \
    "$ROOT_DIR/$source_file" \
    -o "$LINUX32_DIR/$output_name"
}

build_linux_64() {
  local source_file="$1"
  local output_name="$2"

  echo "Building Linux 64-bit: $output_name"

  g++ \
    $CXX_STANDARD \
    $COMMON_FLAGS \
    "$ROOT_DIR/$source_file" \
    -o "$LINUX64_DIR/$output_name"
}

build_all_targets() {
  local source_file="$1"
  local output_name="$2"

  build_windows_32 "$source_file" "$output_name"
  build_windows_64 "$source_file" "$output_name"
  build_linux_32 "$source_file" "$output_name"
  build_linux_64 "$source_file" "$output_name"
}

echo "Starting static builds..."
echo

build_all_targets "caesar.cpp" "csr"
build_all_targets "rail_fence.cpp" "rf"
build_all_targets "row_transposition.cpp" "rt"
build_all_targets "ciphershell.cpp" "shell"

echo
echo "Build complete."
echo
echo "Output structure:"
echo "  output/windows/32-bit/"
echo "    csr.exe"
echo "    rf.exe"
echo "    rt.exe"
echo "    shell.exe"
echo
echo "  output/windows/64-bit/"
echo "    csr.exe"
echo "    rf.exe"
echo "    rt.exe"
echo "    shell.exe"
echo
echo "  output/linux/32-bit/"
echo "    csr"
echo "    rf"
echo "    rt"
echo "    shell"
echo
echo "  output/linux/64-bit/"
echo "    csr"
echo "    rf"
echo "    rt"
echo "    shell"