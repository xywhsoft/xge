#define SOKOL_GLES3
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "../../lib/sokol/sokol_app.h"
#include "../../xge.h"

extern sapp_desc __xgeMakeSokolDesc(void);

static int __xgeIOSFrame(void* pUser)
{
	static int bDumped = 0;
	static int iFrameCount = 0;
	xge_platform_runtime_t tRuntime;
	xge_rect_t tRect;
	char arrCaps[2048];

	(void)pUser;
	iFrameCount++;
	if ( bDumped == 0 ) {
		memset(arrCaps, 0, sizeof(arrCaps));
		(void)xgeDebugDumpCaps(arrCaps, (int)sizeof(arrCaps));
		(void)xgeLogWrite(XGE_LOG_INFO, "platform", arrCaps);
		bDumped = 1;
	}
	if ( (iFrameCount == 1) || ((iFrameCount % 120) == 0) ) {
		memset(&tRuntime, 0, sizeof(tRuntime));
		if ( xgePlatformRuntimeGet(&tRuntime) == XGE_OK ) {
			char arrRuntime[512];
			snprintf(arrRuntime, sizeof(arrRuntime), "Platform Runtime: running=%d window=%dx%d framebuffer=%dx%d dpi=%.2f key=%d text=%d mouse=%d touch=%d gamepad=%d resize=%d quit=%d",
				tRuntime.bRunning,
				tRuntime.iWindowWidth, tRuntime.iWindowHeight,
				tRuntime.iFramebufferWidth, tRuntime.iFramebufferHeight,
				tRuntime.fDpiScale,
				tRuntime.iKeyEventCount, tRuntime.iTextEventCount,
				tRuntime.iMouseEventCount, tRuntime.iTouchEventCount,
				tRuntime.iGamepadEventCount, tRuntime.iResizeEventCount,
				tRuntime.iQuitEventCount);
			(void)xgeLogWrite(XGE_LOG_INFO, "platform", arrRuntime);
		}
	}
	xgeClear(XGE_COLOR_RGBA(24, 32, 48, 255));
	tRect.fX = 24.0f;
	tRect.fY = 24.0f;
	tRect.fW = 180.0f;
	tRect.fH = 96.0f;
	xgeShapeRectFill(tRect, XGE_COLOR_RGBA(64, 128, 220, 255));
	xgeShapeCircleFill(256.0f, 72.0f, 48.0f, XGE_COLOR_RGBA(240, 96, 72, 255));
	return 0;
}

sapp_desc sokol_main(int argc, char* argv[])
{
	xge_desc_t tDesc;

	(void)argc;
	(void)argv;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 1280;
	tDesc.iHeight = 720;
	tDesc.sTitle = "XGE iOS";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	(void)xgeInit(&tDesc);
	(void)xgeRun(__xgeIOSFrame, NULL);
	return __xgeMakeSokolDesc();
}
