#!/bin/sh
set -e

echo "[XGE] Running C language boundary check..."
./check_c_language_boundary.sh

echo
echo "[XGE] Running platform scaffold check..."
./check_platform_scaffold.sh

echo
echo "[XGE] Running script documentation check..."
./check_script_docs.sh

echo
echo "[XGE] Running platform toolchain check..."
./check_platform_backends.sh

echo
echo "[XGE] Running platform result status check..."
if ! ./check_platform_results.sh; then
	echo "[XGE] Platform result check reported blocked platforms. This is expected until real target smoke tests pass."
fi

echo
echo "[XGE] Platform preparation checks finished."
