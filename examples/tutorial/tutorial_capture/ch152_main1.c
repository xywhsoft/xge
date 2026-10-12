/* ch152 — Accordion 手风琴 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_accordion_desc_t desc;
	xui_widget pAccordion;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;
	ret = xuiAccordionCreate(ctx->pContext, &pAccordion, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pAccordion, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pAccordion, (xui_vec2_t){350.0f, 300.0f});
	xuiWidgetAddChild(ctx->pRoot, pAccordion);

	/* 添加折叠面板 */
	xuiAccordionAddSection(pAccordion, "Section 1", 1, 1, NULL);
	xuiAccordionAddSection(pAccordion, "Section 2", 2, 0, NULL);
	xuiAccordionAddSection(pAccordion, "Section 3", 3, 0, NULL);
	return XUI_OK;
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch152", argc, argv); }
