#!/bin/sh
# Build CRISP and run it with Wine. Arguments are passed to build.py (e.g. --config Debug).
set -e
MAC="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(dirname "$MAC")"
CONFIG=Release
prev=""
for arg in "$@"; do
    [ "$prev" = "--config" ] && CONFIG="$arg"
    prev="$arg"
done

python3 "$MAC/build.py" "$@"

# CRISP writes files next to its exe, so it runs from its own folder in the build tree
RUN="$MAC/build/$CONFIG/run"
mkdir -p "$RUN"
cp "$ROOT"/runtime/* "$RUN"/
cp "$MAC/build/$CONFIG/bin/CRISP2.exe" "$MAC/build/$CONFIG/bin/SpinStar.dll" "$MAC/build/$CONFIG/bin/PeakFit.dll" "$RUN"/

export WINEPREFIX="$MAC/build/wineprefix"
export WINEDEBUG="${WINEDEBUG:--all}"
cd "$RUN"
exec "$MAC/tools/wine/Wine Stable.app/Contents/Resources/wine/bin/wine" CRISP2.exe
