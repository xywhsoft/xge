/* ch203 — 实战：矢量动画编辑器 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_toolbar_desc_t tdesc;
	xui_timeline_view_desc_t tldesc;
	xui_canvas_desc_t cdesc;
	xui_widget pToolbar, pTimeline, pCanvas;
	int ret;

	/* 工具栏 */
	memset(&tdesc, 0, sizeof(tdesc));
	tdesc.iSize = sizeof(tdesc);
	tdesc.pFont = ctx->pFont;
	ret = xuiToolbarCreate(ctx->pContext, &pToolbar, &tdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetSizeMode(pToolbar, XUI_SIZE_FILL, XUI_SIZE_CONTENT);
	xuiWidgetAddChild(ctx->pRoot, pToolbar);
	xuiToolbarAddItem(pToolbar, "Select", 0, 0);
	xuiToolbarAddItem(pToolbar, "Pen", 0, 1);
	xuiToolbarAddItem(pToolbar, "Rect", 0, 2);
	xuiToolbarAddSeparator(pToolbar);
	xuiToolbarAddItem(pToolbar, "Play", 0, 3);

	/* 画布 */
	memset(&cdesc, 0, sizeof(cdesc));
	cdesc.iSize = sizeof(cdesc);
	cdesc.fCanvasWidth = 640.0f;
	cdesc.fCanvasHeight = 360.0f;
	ret = xuiCanvasCreate(ctx->pContext, &pCanvas, &cdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetSizeMode(pCanvas, XUI_SIZE_FILL, XUI_SIZE_FILL);
	xuiWidgetAddChild(ctx->pRoot, pCanvas);

	/* 时间轴 */
	memset(&tldesc, 0, sizeof(tldesc));
	tldesc.iSize = sizeof(tldesc);
	tldesc.pFont = ctx->pFont;
	tldesc.iFrameCount = 60;
	tldesc.fFrameRate = 30.0f;
	tldesc.fRowHeight = 28.0f;
	tldesc.fRulerHeight = 20.0f;
	ret = xuiTimeLineViewCreate(ctx->pContext, &pTimeline, &tldesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetSizeMode(pTimeline, XUI_SIZE_FILL, XUI_SIZE_FIXED);
	xuiWidgetSetPreferredSize(pTimeline, (xui_vec2_t){0.0f, 100.0f});
	return xuiWidgetAddChild(ctx->pRoot, pTimeline);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch203", argc, argv); }
