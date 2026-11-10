/* Compile both include orders; function and loader identifiers must coexist. */
#include "xge.h"
#ifdef TEST_SYSTEM_GL_FIRST
#include <GL/gl.h>
#include "src/xge/render/xge_gl.h"
#else
#include "src/xge/render/xge_gl.h"
#include <GL/gl.h>
#endif

#ifdef __cplusplus
#define GL_TEST_ASSERT static_assert
#else
#define GL_TEST_ASSERT _Static_assert
#endif
GL_TEST_ASSERT(sizeof(GLintptr) == sizeof(void*), "GL offsets use the pointer ABI");
GL_TEST_ASSERT(sizeof(GLsizeiptr) == sizeof(void*), "GL sizes use the pointer ABI");
GL_TEST_ASSERT((GLintptr)-1 < 0 && (GLsizeiptr)-1 < 0, "GL offsets and sizes are signed");
#undef GL_TEST_ASSERT
int xge_gl_system_header_coexist(void)
{
    PFNGLVIEWPORTPROC system_viewport = glViewport;
    PFNGLVIEWPORTPROC private_viewport = xge_glViewport;
    return system_viewport != 0 && private_viewport != 0;
}
