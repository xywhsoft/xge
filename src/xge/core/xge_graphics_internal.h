#ifndef XGE_GRAPHICS_INTERNAL_H
#define XGE_GRAPHICS_INTERNAL_H
#include "xge.h"
/* Built-in native owners only. The pair must enclose creation and the actual
 * final storage release, including resources retaining that owner's backend. */
XGE_API int xgeInternalGraphicsOwnerAcquire(void);
XGE_API int xgeInternalGraphicsOwnerRelease(void);
#endif
