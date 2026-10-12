/* ch183 — TimelineView 时间轴 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_timeline_view_desc_t desc;
	xui_widget pTimeline;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;
	desc.iFrameCount = 120;
	desc.fFrameRate = 24.0f;
	desc.iCurrentFrame = 30;
	desc.fFrameWidth = 12.0f;
	desc.fRowHeight = 32.0f;
	desc.fRulerHeight = 24.0f;
	ret = xuiTimeLineViewCreate(ctx->pContext, &pTimeline, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pTimeline, (xui_thickness_t){10.0f, 10.0f, 10.0f, 10.0f});
	xuiWidgetSetSizeMode(pTimeline, XUI_SIZE_FILL, XUI_SIZE_FILL);
	return xuiWidgetAddChild(ctx->pRoot, pTimeline);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch183", argc, argv); }
