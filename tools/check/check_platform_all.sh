#!/bin/sh
set -e

echo "[XGE] Running C language boundary check..."
./tools\check\check_c_language_boundary.sh

echo
echo "[XGE] Running script documentation check..."
./tools\check\check_script_docs.sh

echo
echo "[XGE] Running release documentation check..."
python3 tools/check/check_docs.py

echo
echo "[XGE] Running platform toolchain check..."
./tools\check\check_platform_backends.sh

echo
echo "[XGE] Platform preparation checks finished. Run target smoke tests before release."
