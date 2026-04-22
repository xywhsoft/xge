#include <stdio.h>
#include <string.h>
#include "xge.h"

int main(void)
{
	xge_egl_caps_t tCaps;
	xge_egl_desc_t tDesc;
	xge_egl_context_t tEGL;
	int iRet;

	memset(&tCaps, 0, sizeof(tCaps));
	iRet = xgeEGLCapsGet(&tCaps);
	if ( iRet != XGE_OK ) {
		printf("xgeEGLCapsGet failed: %d\n", iRet);
		return 1;
	}
	printf("EGL backend: %s\n", tCaps.sBackendName);
	printf("compiled=%d pbuffer=%d surfaceless=%d native_window=%d board_linux=%d\n",
		tCaps.bCompiled, tCaps.bPBuffer, tCaps.bSurfaceless, tCaps.bNativeWindow, tCaps.bBoardLinux);
	printf("caps stage=%s error=0x%04X\n", tCaps.sLastStage, (unsigned int)tCaps.iLastError);
	if ( (tCaps.bCompiled == 0) || (tCaps.bPBuffer == 0) ) {
		printf("EGL pbuffer is not enabled in this build.\n");
		return 2;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 64;
	tDesc.iHeight = 64;
	tDesc.bPBuffer = 1;
	tDesc.bBoardLinux = 1;
	memset(&tEGL, 0, sizeof(tEGL));
	iRet = xgeEGLInit(&tEGL, &tDesc);
	if ( iRet != XGE_OK ) {
		printf("xgeEGLInit failed: %d stage=%s error=0x%04X\n", iRet, tEGL.sLastStage, (unsigned int)tEGL.iLastError);
		return 3;
	}
	iRet = xgeEGLMakeCurrent(&tEGL);
	xgeEGLUnit(&tEGL);
	if ( iRet != XGE_OK ) {
		printf("xgeEGLMakeCurrent failed: %d stage=%s error=0x%04X\n", iRet, tEGL.sLastStage, (unsigned int)tEGL.iLastError);
		return 4;
	}
	printf("EGL pbuffer smoke passed.\n");
	return 0;
}
