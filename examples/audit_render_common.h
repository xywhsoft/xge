#ifndef AUDIT_RENDER_COMMON_H
#define AUDIT_RENDER_COMMON_H

#include "../xge.h"
#include <string.h>

static int auditPresentRenderTarget(xge_render_target pTarget, int iWidth, int iHeight, uint32_t iBackgroundColor)
{
	xge_draw_t tDraw;
	xge_texture pTexture;

	if ( (pTarget == NULL) || (iWidth <= 0) || (iHeight <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pTexture = xgeRenderTargetTexture(pTarget);
	if ( pTexture == NULL ) {
		return XGE_ERROR_INVALID_STATE;
	}
	xgeClear(iBackgroundColor);
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = pTexture;
	tDraw.tSrc = (xge_rect_t){0.0f, 0.0f, (float)iWidth, (float)iHeight};
	tDraw.tDst = (xge_rect_t){0.0f, 0.0f, (float)xgeGetWidth(), (float)xgeGetHeight()};
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeDrawEx(&tDraw);
	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	return XGE_OK;
}

#endif
