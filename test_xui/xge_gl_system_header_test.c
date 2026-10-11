/* Compile both include orders; function and loader identifiers must coexist. */
#include "xge.h"
#ifdef TEST_SYSTEM_GL_FIRST
#include <GL/gl.h>
#include "src/xge_gl.h"
#else
#include "src/xge_gl.h"
#include <GL/gl.h>
#endif

_Static_assert(sizeof(GLintptr) == sizeof(void*), "GL offsets use the pointer ABI");
_Static_assert(sizeof(GLsizeiptr) == sizeof(void*), "GL sizes use the pointer ABI");
_Static_assert((GLintptr)-1 < 0 && (GLsizeiptr)-1 < 0, "GL offsets and sizes are signed");
int xge_gl_system_header_coexist(void)
{
    PFNGLVIEWPORTPROC system_viewport = glViewport;
    PFNGLVIEWPORTPROC private_viewport = xge_glViewport;
    return system_viewport != 0 && private_viewport != 0;
}
