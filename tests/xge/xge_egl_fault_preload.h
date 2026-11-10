#ifndef XGE_EGL_FAULT_PRELOAD_H
#define XGE_EGL_FAULT_PRELOAD_H
void xgeEglFaultSet(int mode);
void xgeEglFaultCounts(int* contexts,int* surfaces,int* terminated);
#endif
