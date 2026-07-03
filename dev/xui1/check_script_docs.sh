#!/bin/sh

if command -v python3 >/dev/null 2>&1; then
	python3 tools/check_script_docs.py
	exit $?
fi

if command -v python >/dev/null 2>&1; then
	python tools/check_script_docs.py
	exit $?
fi

echo "[XGE] python3/python not found."
exit 1
