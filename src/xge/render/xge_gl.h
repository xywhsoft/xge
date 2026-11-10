/**
 * @file xge_gl.h
 * @brief OpenGL 3.3 Core 函数加载器 (单头文件实现)
 * 
 * 使用方法:
 *   在一个 .c 文件中定义 XGE_GL_IMPL 后包含此头文件
 *   #define XGE_GL_IMPL
 *   #include "xge/xge_gl.h"
 */

#ifndef XGE_GL_H
#define XGE_GL_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * 平台相关 GL 头文件
 * ============================================================================ */

#if defined(_WIN32)
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
	#ifndef APIENTRY
		#define APIENTRY __stdcall
	#endif
#else
	#ifndef APIENTRY
		#define APIENTRY
	#endif
#endif

#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif

/* ============================================================================
 * GL 类型定义
 * ============================================================================ */

typedef void GLvoid;
typedef unsigned int GLenum;
typedef float GLfloat;
typedef int GLint;
typedef int GLsizei;
typedef unsigned int GLbitfield;
typedef double GLdouble;
typedef unsigned int GLuint;
typedef unsigned char GLboolean;
typedef unsigned char GLubyte;
typedef char GLchar;
typedef short GLshort;
typedef signed char GLbyte;
typedef unsigned short GLushort;
/* GL buffer offsets follow the platform pointer ABI (LP64 and LLP64). */
typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;
typedef float GLclampf;

/* ============================================================================
 * GL 常量定义
 * ============================================================================ */

#define GL_FALSE 0
#define GL_TRUE 1
#define GL_NONE 0
#define GL_ZERO 0
#define GL_ONE 1

/* 数据类型 */
#define GL_BYTE 0x1400
#define GL_UNSIGNED_BYTE 0x1401
#define GL_SHORT 0x1402
#define GL_UNSIGNED_SHORT 0x1403
#define GL_INT 0x1404
#define GL_UNSIGNED_INT 0x1405
#define GL_FLOAT 0x1406

/* 图元类型 */
#define GL_POINTS 0x0000
#define GL_LINES 0x0001
#define GL_LINE_LOOP 0x0002
#define GL_LINE_STRIP 0x0003
#define GL_TRIANGLES 0x0004
#define GL_TRIANGLE_STRIP 0x0005
#define GL_TRIANGLE_FAN 0x0006

/* 缓冲对象 */
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_STATIC_DRAW 0x88E4
#define GL_DYNAMIC_DRAW 0x88E8

/* 清除缓冲 */
#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_STENCIL_BUFFER_BIT 0x00000400
#define GL_COLOR_BUFFER_BIT 0x00004000

/* 开启/关闭 */
#define GL_BLEND 0x0BE2
#define GL_CULL_FACE 0x0B44
#define GL_DEPTH_TEST 0x0B71
#define GL_STENCIL_TEST 0x0B90
#define GL_SCISSOR_TEST 0x0C11

/* 混合 */
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_FUNC_ADD 0x8006
#define GL_MIN 0x8007
#define GL_MAX 0x8008

/* 深度 */
#define GL_ALWAYS 0x0207
#define GL_EQUAL 0x0202
#define GL_LESS 0x0201
#define GL_LEQUAL 0x0203

#define GL_KEEP 0x1E00
#define GL_REPLACE 0x1E01

/* 面剔除 */
#define GL_FRONT 0x0404
#define GL_BACK 0x0405
#define GL_CW 0x0900
#define GL_CCW 0x0901

/* 纹理 */
#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_UNPACK_ALIGNMENT 0x0CF5
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_LINEAR_MIPMAP_LINEAR 0x2703
#define GL_REPEAT 0x2901
#define GL_CLAMP_TO_EDGE 0x812F

/* 像素格式 */
#define GL_RED 0x1903
#define GL_RGB 0x1907
#define GL_RGBA 0x1908
#define GL_RGB8 0x8051
#define GL_R8 0x8229
#define GL_RGBA8 0x8058
#define GL_RG 0x8227
#define GL_RG8 0x822B
#define GL_GREEN 0x1904
#define GL_BLUE 0x1905
#define GL_ALPHA 0x1906
#define GL_RGB565 0x8D62
#define GL_RGB5_A1 0x8057
#define GL_RGBA4 0x8056
#define GL_UNSIGNED_SHORT_5_6_5 0x8363
#define GL_UNSIGNED_SHORT_5_5_5_1 0x8034
#define GL_UNSIGNED_SHORT_4_4_4_4 0x8033
#define GL_TEXTURE_BINDING_2D 0x8069
#define GL_TEXTURE_SWIZZLE_R 0x8E42
#define GL_TEXTURE_SWIZZLE_G 0x8E43
#define GL_TEXTURE_SWIZZLE_B 0x8E44
#define GL_TEXTURE_SWIZZLE_A 0x8E45

/* 帧缓冲 */
#define GL_FRAMEBUFFER 0x8D40
#define GL_RENDERBUFFER 0x8D41
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_DEPTH_ATTACHMENT 0x8D00
#define GL_STENCIL_ATTACHMENT 0x8D20
#define GL_STENCIL_INDEX8 0x8D48
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5

/* 着色器 */
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84

/* 获取信息 */
#define GL_VENDOR 0x1F00
#define GL_RENDERER 0x1F01
#define GL_VERSION 0x1F02
/* GLES3 / desktop GL3 context validation and viewport limits. */
#define GL_MAJOR_VERSION 0x821B
#define GL_MINOR_VERSION 0x821C
#define GL_MAX_VIEWPORT_DIMS 0x0D3A
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C

/* 错误 */
#define GL_NO_ERROR 0
#define GL_INVALID_ENUM 0x0500
#define GL_INVALID_VALUE 0x0501
#define GL_INVALID_OPERATION 0x0502
#define GL_OUT_OF_MEMORY 0x0505

/* ============================================================================
 * GL 函数指针类型定义
 * ============================================================================ */

typedef void (APIENTRYP PFNGLVIEWPORTPROC)(GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLCLEARPROC)(GLbitfield mask);
typedef void (APIENTRYP PFNGLCLEARCOLORPROC)(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
typedef void (APIENTRYP PFNGLFLUSHPROC)(void);
typedef void (APIENTRYP PFNGLENABLEPROC)(GLenum cap);
typedef void (APIENTRYP PFNGLDISABLEPROC)(GLenum cap);
typedef void (APIENTRYP PFNGLSCISSORPROC)(GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLREADPIXELSPROC)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);
typedef void (APIENTRYP PFNGLBLENDFUNCPROC)(GLenum sfactor, GLenum dfactor);
typedef void (APIENTRYP PFNGLBLENDEQUATIONPROC)(GLenum mode);
typedef void (APIENTRYP PFNGLDEPTHFUNCPROC)(GLenum func);
#if XGE_ENABLE_3D
#if XGE3D_ENABLE_SHADOW
typedef void (APIENTRYP PFNGLDRAWBUFFERSPROC)(GLsizei n, const GLenum *buffers);
#endif
typedef void (APIENTRYP PFNGLREADBUFFERPROC)(GLenum buffer);
typedef void (APIENTRYP PFNGLDEPTHMASKPROC)(GLboolean flag);
typedef void (APIENTRYP PFNGLCLEARDEPTHPROC)(GLdouble depth);
typedef void (APIENTRYP PFNGLCLEARDEPTHFPROC)(GLfloat depth);
typedef void (APIENTRYP PFNGLGETFLOATVPROC)(GLenum pname, GLfloat *data);
typedef void (APIENTRYP PFNGLBLENDFUNCSEPARATEPROC)(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
typedef void (APIENTRYP PFNGLBLENDEQUATIONSEPARATEPROC)(GLenum rgb, GLenum alpha);
#endif
typedef void (APIENTRYP PFNGLCULLFACEPROC)(GLenum mode);
typedef void (APIENTRYP PFNGLFRONTFACEPROC)(GLenum mode);
typedef GLenum (APIENTRYP PFNGLGETERRORPROC)(void);
typedef const GLubyte* (APIENTRYP PFNGLGETSTRINGPROC)(GLenum name);
typedef void (APIENTRYP PFNGLGETINTEGERVPROC)(GLenum pname, GLint *data);
typedef void (APIENTRYP PFNGLCLEARSTENCILPROC)(GLint s);
typedef void (APIENTRYP PFNGLSTENCILFUNCPROC)(GLenum func, GLint ref, GLuint mask);
typedef void (APIENTRYP PFNGLSTENCILOPPROC)(GLenum sfail, GLenum dpfail, GLenum dppass);
typedef void (APIENTRYP PFNGLSTENCILMASKPROC)(GLuint mask);
typedef void (APIENTRYP PFNGLCOLORMASKPROC)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);

/* 缓冲 */
typedef void (APIENTRYP PFNGLGENBUFFERSPROC)(GLsizei n, GLuint *buffers);
typedef void (APIENTRYP PFNGLDELETEBUFFERSPROC)(GLsizei n, const GLuint *buffers);
typedef void (APIENTRYP PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
typedef void (APIENTRYP PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void (APIENTRYP PFNGLBUFFERSUBDATAPROC)(GLenum target, GLintptr offset, GLsizeiptr size, const void *data);

/* 顶点数组 */
typedef void (APIENTRYP PFNGLGENVERTEXARRAYSPROC)(GLsizei n, GLuint *arrays);
typedef void (APIENTRYP PFNGLDELETEVERTEXARRAYSPROC)(GLsizei n, const GLuint *arrays);
typedef void (APIENTRYP PFNGLBINDVERTEXARRAYPROC)(GLuint array);
typedef void (APIENTRYP PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint index);
typedef void (APIENTRYP PFNGLVERTEXATTRIBPOINTERPROC)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);

/* 绘制 */
typedef void (APIENTRYP PFNGLDRAWARRAYSPROC)(GLenum mode, GLint first, GLsizei count);
typedef void (APIENTRYP PFNGLDRAWELEMENTSPROC)(GLenum mode, GLsizei count, GLenum type, const void *indices);
#if XGE_ENABLE_3D
typedef void (APIENTRYP PFNGLDRAWARRAYSINSTANCEDPROC)(GLenum mode, GLint first, GLsizei count, GLsizei instances);
typedef void (APIENTRYP PFNGLDRAWELEMENTSINSTANCEDPROC)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instances);
#endif

/* 纹理 */
typedef void (APIENTRYP PFNGLGENTEXTURESPROC)(GLsizei n, GLuint *textures);
typedef void (APIENTRYP PFNGLDELETETEXTURESPROC)(GLsizei n, const GLuint *textures);
typedef void (APIENTRYP PFNGLBINDTEXTUREPROC)(GLenum target, GLuint texture);
typedef void (APIENTRYP PFNGLACTIVETEXTUREPROC)(GLenum texture);
typedef void (APIENTRYP PFNGLTEXIMAGE2DPROC)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
typedef void (APIENTRYP PFNGLTEXSUBIMAGE2DPROC)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
typedef void (APIENTRYP PFNGLCOPYTEXSUBIMAGE2DPROC)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLPIXELSTOREIPROC)(GLenum pname, GLint param);
typedef void (APIENTRYP PFNGLTEXPARAMETERIPROC)(GLenum target, GLenum pname, GLint param);
typedef void (APIENTRYP PFNGLGENERATEMIPMAPPROC)(GLenum target);

/* 着色器 */
typedef GLuint (APIENTRYP PFNGLCREATESHADERPROC)(GLenum type);
typedef void (APIENTRYP PFNGLDELETESHADERPROC)(GLuint shader);
typedef void (APIENTRYP PFNGLSHADERSOURCEPROC)(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
typedef void (APIENTRYP PFNGLCOMPILESHADERPROC)(GLuint shader);
typedef void (APIENTRYP PFNGLGETSHADERIVPROC)(GLuint shader, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETSHADERINFOLOGPROC)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);

/* 程序 */
typedef GLuint (APIENTRYP PFNGLCREATEPROGRAMPROC)(void);
typedef void (APIENTRYP PFNGLDELETEPROGRAMPROC)(GLuint program);
typedef void (APIENTRYP PFNGLATTACHSHADERPROC)(GLuint program, GLuint shader);
typedef void (APIENTRYP PFNGLLINKPROGRAMPROC)(GLuint program);
typedef void (APIENTRYP PFNGLUSEPROGRAMPROC)(GLuint program);
typedef void (APIENTRYP PFNGLGETPROGRAMIVPROC)(GLuint program, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETPROGRAMINFOLOGPROC)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef GLint (APIENTRYP PFNGLGETUNIFORMLOCATIONPROC)(GLuint program, const GLchar *name);

/* Uniform */
typedef void (APIENTRYP PFNGLUNIFORM1IPROC)(GLint location, GLint v0);
typedef void (APIENTRYP PFNGLUNIFORM1FPROC)(GLint location, GLfloat v0);
typedef void (APIENTRYP PFNGLUNIFORM2FPROC)(GLint location, GLfloat v0, GLfloat v1);
typedef void (APIENTRYP PFNGLUNIFORM3FPROC)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (APIENTRYP PFNGLUNIFORM4FPROC)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX4FVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

/* 帧缓冲 */
typedef void (APIENTRYP PFNGLGENFRAMEBUFFERSPROC)(GLsizei n, GLuint *framebuffers);
typedef void (APIENTRYP PFNGLDELETEFRAMEBUFFERSPROC)(GLsizei n, const GLuint *framebuffers);
typedef void (APIENTRYP PFNGLBINDFRAMEBUFFERPROC)(GLenum target, GLuint framebuffer);
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE2DPROC)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef GLenum (APIENTRYP PFNGLCHECKFRAMEBUFFERSTATUSPROC)(GLenum target);
#if XGE_ENABLE_3D
typedef void (APIENTRYP PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)(GLenum target, GLenum attachment, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLBLITFRAMEBUFFERPROC)(GLint sx0,GLint sy0,GLint sx1,GLint sy1,GLint dx0,GLint dy0,GLint dx1,GLint dy1,GLbitfield mask,GLenum filter);
#endif
typedef void (APIENTRYP PFNGLGENRENDERBUFFERSPROC)(GLsizei n, GLuint *renderbuffers);
typedef void (APIENTRYP PFNGLDELETERENDERBUFFERSPROC)(GLsizei n, const GLuint *renderbuffers);
typedef void (APIENTRYP PFNGLBINDRENDERBUFFERPROC)(GLenum target, GLuint renderbuffer);
typedef void (APIENTRYP PFNGLRENDERBUFFERSTORAGEPROC)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLFRAMEBUFFERRENDERBUFFERPROC)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);

/* ============================================================================
 * GL 加载函数
 * ============================================================================ */

#if XGE_ENABLE_3D
/* Optional desktop 3.3 timestamps; absent on GLES/WebGL without this path. */
typedef void (APIENTRYP D3GLGENQUERIESPROC)(GLsizei,GLuint*);
typedef void (APIENTRYP D3GLDELETEQUERIESPROC)(GLsizei,const GLuint*);
typedef void (APIENTRYP D3GLQUERYCOUNTERPROC)(GLuint,GLenum);
typedef void (APIENTRYP D3GLGETQUERYOBJECTIVPROC)(GLuint,GLenum,GLint*);
typedef void (APIENTRYP D3GLGETQUERYOBJECTUI64VPROC)(GLuint,GLenum,uint64_t*);
#endif

#define XGE_GL_SLOT(Type,Name,Required) extern Type xge_##Name;
#include "xge_gl_slots.inc"
#undef XGE_GL_SLOT

typedef void* (*XgeGLLoadProc)(const char *name);
typedef void* (*XgeGLLoadProcEx)(const char* name,void* user);

/**
 * @brief 加载 GL 函数
 * @param procLoad 函数指针加载器
 * @return 成功返回1, 失败返回0
 */
int xge_gl_load(XgeGLLoadProc procLoad);
/* Transactional loading: failure leaves every previously committed slot intact. */
int xge_gl_load_ex(XgeGLLoadProcEx load,void* user);
void xge_gl_unload(void);

#ifdef __cplusplus
}
#endif

/* ============================================================================
 * 实现部分
 * ============================================================================ */

#ifdef XGE_GL_IMPL

#define XGE_GL_SLOT(Type,Name,Required) Type xge_##Name=NULL;
#include "xge_gl_slots.inc"
#undef XGE_GL_SLOT

int xge_gl_load_ex(XgeGLLoadProcEx load,void* user)
{
    if(!load)return 0;
#define XGE_GL_SLOT(Type,Name,Required) Type pending_##Name=(Type)load(#Name,user); if((Required) && !pending_##Name)return 0;
#include "xge_gl_slots.inc"
#undef XGE_GL_SLOT
#if XGE_ENABLE_3D
    if(!pending_glClearDepth && !pending_glClearDepthf)return 0;
#endif
#define XGE_GL_SLOT(Type,Name,Required) xge_##Name=pending_##Name;
#include "xge_gl_slots.inc"
#undef XGE_GL_SLOT
    return 1;
}
static void* __xgeGlLegacyLoad(const char* name,void* user)
{return (*(XgeGLLoadProc*)user)(name);}
int xge_gl_load(XgeGLLoadProc load)
{return load ? xge_gl_load_ex(__xgeGlLegacyLoad,&load):0;}
void xge_gl_unload(void)
{
#define XGE_GL_SLOT(Type,Name,Required) xge_##Name=NULL;
#include "xge_gl_slots.inc"
#undef XGE_GL_SLOT
}

#endif /* XGE_GL_IMPL */

#endif /* XGE_GL_H */
