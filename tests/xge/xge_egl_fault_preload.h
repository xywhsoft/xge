#ifndef XGE_EGL_FAULT_PRELOAD_H
#define XGE_EGL_FAULT_PRELOAD_H
void xgeEglFaultSet(int mode);
void xgeEglFaultCounts(int* contexts,int* surfaces,int* terminated);
/* Fail one cleanup attempt: 1=unbind, 2=context, 3=surface, 4=display.
 * Counters retain successful native releases across arming and retry. */
void xgeEglCleanupFaultArm(int stage);
#endif
