#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="$ROOT/build"

mkdir -p "$BUILD/tmp"
export TMP="$BUILD/tmp"
export TEMP="$BUILD/tmp"

if [ ! -f "$BUILD/CMakeCache.txt" ]; then
    echo "==> Configuring"
    cmake -S "$ROOT" -B "$BUILD"
fi

echo "==> Building"
cmake --build "$BUILD"

echo "==> Done"
