/**
 * @file xge_platform.h
 * @brief 平台检测和配置宏
 */

#ifndef XGE_PLATFORM_H
#define XGE_PLATFORM_H

/* ============================================================================
 * 平台检测
 * ============================================================================ */

#if defined(_WIN32) || defined(_WIN64)
	#define XGE_PLATFORM_WINDOWS
	#define XGE_PLATFORM_NAME "Windows"
#elif defined(__APPLE__) && defined(__MACH__)
	#include <TargetConditionals.h>
	#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
		#define XGE_PLATFORM_IOS
		#define XGE_PLATFORM_NAME "iOS"
	#else
		#define XGE_PLATFORM_MACOS
		#define XGE_PLATFORM_NAME "macOS"
	#endif
#elif defined(__ANDROID__)
	#define XGE_PLATFORM_ANDROID
	#define XGE_PLATFORM_NAME "Android"
#elif defined(__EMSCRIPTEN__)
	#define XGE_PLATFORM_WEB
	#define XGE_PLATFORM_NAME "Web"
#elif defined(__linux__)
	#define XGE_PLATFORM_LINUX
	#define XGE_PLATFORM_NAME "Linux"
#else
	#error "Unsupported platform"
#endif

/* ============================================================================
 * OpenGL 版本配置
 * ============================================================================ */

#if defined(XGE_PLATFORM_WINDOWS) || defined(XGE_PLATFORM_LINUX) || defined(XGE_PLATFORM_MACOS)
	/* 桌面端: OpenGL 3.3 Core Profile */
	#define XGE_GL_MAJOR_VERSION 3
	#define XGE_GL_MINOR_VERSION 3
	#define XGE_USE_GLES 0
	#define XGE_GLSL_VERSION "#version 330 core\n"
#else
	/* 移动端/Web: OpenGL ES 3.0 */
	#define XGE_GL_MAJOR_VERSION 3
	#define XGE_GL_MINOR_VERSION 0
	#define XGE_USE_GLES 1
	#define XGE_GLSL_VERSION "#version 300 es\nprecision mediump float;\n"
#endif

/* ============================================================================
 * Sokol 后端配置
 * ============================================================================ */

#if defined(XGE_PLATFORM_WINDOWS) || defined(XGE_PLATFORM_LINUX) || defined(XGE_PLATFORM_MACOS)
	#define SOKOL_GLCORE
#elif defined(XGE_PLATFORM_IOS) || defined(XGE_PLATFORM_ANDROID)
	#define SOKOL_GLES3
#elif defined(XGE_PLATFORM_WEB)
	#define SOKOL_GLES3
#endif

/* ============================================================================
 * 编译器检测
 * ============================================================================ */

#if defined(_MSC_VER)
	#define XGE_COMPILER_MSVC
	#define XGE_COMPILER_NAME "MSVC"
#elif defined(__TINYC__)
	#define XGE_COMPILER_TCC
	#define XGE_COMPILER_NAME "TCC"
#elif defined(__clang__)
	#define XGE_COMPILER_CLANG
	#define XGE_COMPILER_NAME "Clang"
#elif defined(__GNUC__)
	#define XGE_COMPILER_GCC
	#define XGE_COMPILER_NAME "GCC"
#else
	#define XGE_COMPILER_UNKNOWN
	#define XGE_COMPILER_NAME "Unknown"
#endif

/* ============================================================================
 * 通用类型定义
 * ============================================================================ */

#include <stdint.h>
#include <stddef.h>

typedef int32_t  xge_bool;
typedef int32_t  xge_int;
typedef uint32_t xge_uint;
typedef float    xge_float;

#define XGE_TRUE  1
#define XGE_FALSE 0
#define XGE_NULL  ((void*)0)

/* ============================================================================
 * 调试宏
 * ============================================================================ */

#ifdef XGE_DEBUG
	#include <stdio.h>
	#define XGE_LOG(fmt, ...) fprintf(stdout, "[XGE] " fmt "\n", ##__VA_ARGS__)
	#define XGE_ERROR(fmt, ...) fprintf(stderr, "[XGE ERROR] " fmt "\n", ##__VA_ARGS__)
#else
	#define XGE_LOG(fmt, ...) ((void)0)
	#define XGE_ERROR(fmt, ...) ((void)0)
#endif

#endif /* XGE_PLATFORM_H */
