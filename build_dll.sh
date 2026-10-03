#!/bin/sh
set -e
cd "$(dirname "$0")"
exec python3 tools/build_profile.py "$@"
