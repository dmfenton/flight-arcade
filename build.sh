#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$ROOT"
GBDK_ROOT=${GBDK_ROOT:-"$ROOT/tools/gbdk"}
case "$GBDK_ROOT" in /*) ;; *) GBDK_ROOT="$ROOT/$GBDK_ROOT" ;; esac
LCC="$GBDK_ROOT/bin/lcc"
if [ ! -x "$LCC" ]; then
    printf 'GBDK compiler not found at %s. See %s/README.md for setup.\n' "$LCC" "$ROOT" >&2
    exit 1
fi
python3 "$ROOT/src/generate_assets.py"
python3 "$ROOT/src/generate_catalog.py"
python3 "$ROOT/src/generate_puzzles.py"
mkdir -p "$ROOT/build"
for src in "$ROOT"/src/*.c; do
    name=$(basename "$src" .c)
    if [ ! -f "$ROOT/build/$name.o" ] || [ "$src" -nt "$ROOT/build/$name.o" ] || [ "$ROOT/src/arcade.h" -nt "$ROOT/build/$name.o" ] || [ "$ROOT/src/catalog.h" -nt "$ROOT/build/$name.o" ]; then
        "$LCC" -Wa-l -Wl-m -Wf--max-allocs-per-node50000 -c -o "$ROOT/build/$name.o" "$src"
    fi
done
"$LCC" -Wa-l -Wl-m -Wl-j -Wm-yC -Wm-yt0x1B -Wm-ya4 -Wm-yo32 -Wm-ynFLIGHTARCADE -o "$ROOT/build/FlightArcade.gbc" "$ROOT"/build/*.o
printf 'Built %s\n' "$ROOT/build/FlightArcade.gbc"
