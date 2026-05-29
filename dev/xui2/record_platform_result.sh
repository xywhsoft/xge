#!/bin/sh

if command -v python3 >/dev/null 2>&1; then
	python3 tools/record_platform_result.py "$@"
	exit $?
fi

if command -v python >/dev/null 2>&1; then
	python tools/record_platform_result.py "$@"
	exit $?
fi

echo "[XGE] python3/python not found."
exit 1
