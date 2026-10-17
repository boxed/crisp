#!/bin/sh
# One-time setup for building CRISP on macOS. See mac/README.md.
set -e
MAC="$(cd "$(dirname "$0")" && pwd)"
TOOLS="$MAC/tools"
mkdir -p "$TOOLS/bin"

for formula in llvm@22 lld@22; do
    if [ ! -d "/opt/homebrew/opt/$formula" ]; then
        echo "Missing $formula: brew install $formula"
        exit 1
    fi
done

# JWasm: MASM-compatible assembler for the .ass files (llvm-ml can't handle MASM 6 syntax)
if [ ! -x "$TOOLS/bin/jwasm" ]; then
    rm -rf "$TOOLS/JWasm"
    git clone -q https://github.com/Baron-von-Riedesel/JWasm.git "$TOOLS/JWasm"
    git -C "$TOOLS/JWasm" checkout -q 7f6f32e
    mkdir -p "$TOOLS/shim"
    # macOS has no malloc.h
    printf '#include <stdlib.h>\n' > "$TOOLS/shim/malloc.h"
    # The makefile's final link uses a GNU-only -Map flag and fails; link the objects by hand
    make -C "$TOOLS/JWasm" -f CLUnix.mak -j4 inc_dirs="-Isrc/H -I../shim" > /dev/null 2>&1 || true
    clang "$TOOLS/JWasm"/build/CLUnixR/*.o -o "$TOOLS/bin/jwasm"
    echo "Built $TOOLS/bin/jwasm"
fi

# Wine (WineHQ stable build for macOS, the one Homebrew's disabled wine-stable cask used)
if [ ! -d "$TOOLS/wine/Wine Stable.app" ]; then
    mkdir -p "$TOOLS/wine"
    curl -fsSL https://github.com/Gcenx/macOS_Wine_builds/releases/download/11.0_1/wine-stable-11.0_1-osx64.tar.xz \
        | tar xJ -C "$TOOLS/wine"
    echo "Installed Wine into $TOOLS/wine"
fi

# VS2008 headers and libraries cannot be downloaded; they are copied from a VS2008 install
VS="$MAC/vs2008"
for d in vc/include vc/lib vc/atlmfc/include vc/atlmfc/lib sdk/include sdk/lib; do
    if [ ! -d "$VS/$d" ]; then
        echo "Missing $VS/$d (see mac/README.md)"
        exit 1
    fi
done
# MFC's .rc files use "res\\file" paths, which llvm-rc only understands on Windows
python3 - "$VS/vc/atlmfc/include" <<'EOF'
import sys
from pathlib import Path
for p in Path(sys.argv[1]).rglob('*.rc'):
    s = p.read_bytes()
    if b'"res\\\\' in s:
        p.write_bytes(s.replace(b'"res\\\\', b'"res/'))
EOF

echo "Setup complete. Build and run with mac/run.sh"
