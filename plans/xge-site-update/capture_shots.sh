#!/bin/bash
# M5a: build all 3D/document examples and capture tutorial screenshots.
# ASCII console output only.
export PATH="/e/software/w64devkit/bin:$PATH"
cd /d/GIT/xge || exit 1
OUT=artifacts/site-shots
mkdir -p "$OUT"
log() { echo "[$(date +%H:%M:%S)] $*"; }

log "STEP1 build 3d profile"
python tools/build_profile.py 3d || { echo "FAIL step1"; exit 1; }

log "STEP2 build simple 3d exes"
gcc -O2 -Wall -Wextra -Werror -Wno-missing-field-initializers -DXGE_DLL -I. \
  -include build/3d/xge_build_config.h examples/xge_3d/main.c build/3d/xge.lib -lm \
  -o build/3d/xge_3d.exe || { echo "FAIL xge_3d"; exit 1; }
python test/make_3d_fixtures.py || { echo "FAIL fixtures"; exit 1; }
gcc -O2 -Wall -Wextra -Werror -Wno-missing-field-initializers -DXGE_DLL -I. \
  -include build/3d/xge_build_config.h examples/xge_3d_scene/main.c build/3d/xge.lib -lm \
  -o build/3d/xge_3d_scene.exe || { echo "FAIL xge_3d_scene"; exit 1; }
python test/build_3d_suite.py ibl 3d || { echo "FAIL ibl suite"; exit 1; }
gcc -O2 -Wall -Wextra -Werror -Wno-missing-field-initializers -DXGE_DLL -I. \
  -include build/3d/xge_build_config.h examples/xge_3d_lighting/main.c build/3d/xge.lib -lm \
  -o build/3d/xge_3d_lighting.exe || { echo "FAIL xge_3d_lighting"; exit 1; }
python test/test_3d_motion.py >/dev/null 2>&1 || { echo "FAIL motion"; exit 1; }
python test/test_3d_retarget.py >/dev/null 2>&1 || { echo "FAIL retarget"; exit 1; }
gcc -O2 -Wall -Wextra -Werror -Wno-missing-field-initializers -DXGE_DLL -I. \
  -include build/3d/xge_build_config.h examples/xge_3d_animation/main.c build/3d/xge.lib -lm \
  -o build/3d/xge_3d_animation.exe || { echo "FAIL xge_3d_animation"; exit 1; }

log "STEP3 build walk demo"
python examples/xge_3d_walk/build.py || { echo "FAIL walk build"; exit 1; }
WALKBUILD=$(ls -d build/xge-3d-walk build/xge_3d_walk 2>/dev/null | head -1)
log "walk build dir: $WALKBUILD"

log "STEP4 build integration (full-dev)"
python examples/xge_3d_integration/build.py full-dev || { echo "FAIL integration build"; exit 1; }

log "STEP5 build xui_document"
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -I. -o build/xui_document.exe \
  examples/xui_document/main.c build/xge.lib -lm -lws2_32 -liphlpapi -lgdi32 -luser32 \
  -lshell32 -lole32 -lwinmm -lavrt || { echo "FAIL xui_document"; exit 1; }

log "STEP6 captures"
cap() { # exe args... out
  local out="$1"; shift
  "$@" || { echo "FAIL run $out"; return 1; }
}
build/3d/xge_3d_scene.exe      --frames 100 --capture "$OUT/ch212.png"   && log ch212 ok
build/3d/xge_3d.exe            --frames 160 --capture "$OUT/ch213.png"   && log ch213 ok
"$WALKBUILD/xge_3d_walk.exe"   --frames 120 --seed 7  --capture "$OUT/ch214.png"  && log ch214 ok
"$WALKBUILD/xge_3d_walk.exe"   --frames 240 --seed 21 --capture "$OUT/ch215.png"  && log ch215 ok
build/3d/xge_3d_lighting.exe   --frames 100 --capture "$OUT/ch216.png"   && log ch216 ok
build/3d/xge_3d_lighting.exe   --frames 260 --capture "$OUT/ch217.png"   && log ch217 ok
build/3d/xge_3d_lighting.exe   --frames 420 --ibl --capture "$OUT/ch218.png" && log ch218 ok
build/3d/xge_3d_animation.exe  --frames 200 --capture "$OUT/ch219.png"   && log ch219 ok
"$WALKBUILD/xge_3d_walk.exe"   --frames 60  --seed 3  --capture "$OUT/ch220.png"  && log ch220 ok
"$WALKBUILD/xge_3d_walk.exe"   --frames 400 --seed 11 --capture "$OUT/ch221.png"  && log ch221 ok
python examples/xge_3d_integration/build.py full-dev --run --frames 200 --capture "$OUT/ch222.png" && log ch222 ok
"$WALKBUILD/xge_3d_walk.exe"   --frames 150 --seed 5 --first-person --capture "$OUT/ch223.png" && log ch223 ok
"$WALKBUILD/xge_3d_walk.exe"   --frames 300 --capture "$OUT/ch224.png"   && log ch224 ok
python examples/xge_3d_integration/build.py full-dev --run --frames 420 --capture "$OUT/ch225.png" && log ch225 ok
rm -rf artifacts/xui-document-rebuild
./build/xui_document.exe --verify >/dev/null 2>&1 && cp artifacts/xui-document-rebuild/native-smoke.png "$OUT/ch226.png" && log ch226 ok
./build/xui_document.exe --verify >/dev/null 2>&1 && cp artifacts/xui-document-rebuild/native-live-smoke.png "$OUT/ch227.png" && log ch227 ok

log "DONE"
ls -la "$OUT"
