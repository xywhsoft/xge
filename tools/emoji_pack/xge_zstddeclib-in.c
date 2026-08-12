/**
 * XGE private Zstandard decompressor.
 *
 * Generated from the official Zstandard single-file decompressor. The selected
 * decoder variants are fixed because XGE's built-in Emoji package is generated
 * and validated together with this implementation.
 */
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * accompanying lib/zstd/LICENSE file.
 */

#define DEBUGLEVEL 0
#define ZSTD_ASSERTIONS 0
#define NDEBUG

#define MEM_MODULE
#undef XXH_NAMESPACE
#define XXH_NAMESPACE ZSTD_
#undef XXH_PRIVATE_API
#define XXH_PRIVATE_API
#undef XXH_INLINE_ALL
#define XXH_INLINE_ALL

#define ZSTD_LEGACY_SUPPORT 0
#define ZSTD_TRACE 0
#define ZSTD_DISABLE_ASM 1
#define ZSTD_STRIP_ERROR_STRINGS 1
#define HUF_FORCE_DECOMPRESS_X1 1
#define ZSTD_FORCE_DECOMPRESS_SEQUENCES_SHORT 1
#define ZSTDLIB_VISIBILITY static

#define ZSTD_DEPS_NEED_MALLOC
#include "common/zstd_deps.h"

#include "common/debug.c"
#include "common/entropy_common.c"
#include "common/error_private.c"
#include "common/fse_decompress.c"
#include "common/zstd_common.c"

#include "decompress/huf_decompress.c"
#include "decompress/zstd_ddict.c"
#include "decompress/zstd_decompress.c"
#include "decompress/zstd_decompress_block.c"
