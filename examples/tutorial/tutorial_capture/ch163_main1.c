/* ch163 — Carousel 轮播 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_carousel_desc_t desc;
	xui_widget pCarousel;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;
	desc.iPageCount = 4;
	desc.iCurrent = 0;
	desc.bLoop = 1;
	desc.bShowIndicators = 1;
	desc.bShowArrowsOnHover = 1;
	ret = xuiCarouselCreate(ctx->pContext, &pCarousel, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pCarousel, (xui_thickness_t){40.0f, 40.0f, 40.0f, 40.0f});
	xuiWidgetSetPreferredSize(pCarousel, (xui_vec2_t){600.0f, 300.0f});
	return xuiWidgetAddChild(ctx->pRoot, pCarousel);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch163", argc, argv); }
