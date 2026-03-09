#!/bin/bash

# ============================================================================
# XGE 引擎编译脚本 - Linux
# ============================================================================

SRC="main.c"
OUT="xge"
INC="-I."
LIBS="-lX11 -lXi -lXcursor -lGL -ldl -lpthread -lm"
FLAGS="-O2 -Wall"

# 检查编译器
if ! command -v gcc &> /dev/null; then
    echo "[ERROR] gcc not found"
    exit 1
fi

echo "[XGE] Compiling $SRC..."
gcc $FLAGS $INC -o $OUT $SRC $LIBS

if [ $? -eq 0 ]; then
    echo "[XGE] Build successful: $OUT"
    chmod +x $OUT
else
    echo "[XGE] Build failed"
    exit 1
fi
