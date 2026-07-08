#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="$ROOT/build"

echo "==> Cleaning $BUILD"
rm -rf "$BUILD"

mkdir -p "$BUILD/tmp"
export TMP="$BUILD/tmp"
export TEMP="$BUILD/tmp"

echo "==> Configuring"
cmake -S "$ROOT" -B "$BUILD"

echo "==> Building"
cmake --build "$BUILD"

echo "==> Done"
