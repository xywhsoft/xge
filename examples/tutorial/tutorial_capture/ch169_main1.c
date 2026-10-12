/* ch169 — Slider 滑块 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_slider_desc_t desc;
	xui_widget pSlider;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.fMin = 0.0f;
	desc.fMax = 100.0f;
	desc.fValue = 60.0f;
	desc.fStep = 1.0f;
	desc.iOrientation = 0; /* horizontal */
	desc.fTrackSize = 6.0f;
	desc.fKnobSize = 18.0f;
	ret = xuiSliderCreate(ctx->pContext, &pSlider, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pSlider, (xui_thickness_t){40.0f, 60.0f, 40.0f, 0.0f});
	xuiWidgetSetPreferredSize(pSlider, (xui_vec2_t){400.0f, 30.0f});
	xuiWidgetAddChild(ctx->pRoot, pSlider);

	/* 第二个滑块 — 不同值 */
	desc.fValue = 25.0f;
	ret = xuiSliderCreate(ctx->pContext, &pSlider, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pSlider, (xui_thickness_t){40.0f, 20.0f, 40.0f, 0.0f});
	xuiWidgetSetPreferredSize(pSlider, (xui_vec2_t){400.0f, 30.0f});
	return xuiWidgetAddChild(ctx->pRoot, pSlider);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch169", argc, argv); }
