/* ch151 — Tabs 标签页 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_tabs_desc_t desc;
	xui_label_desc_t ldesc;
	xui_widget pTabs, pPage, pLabel;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;
	ret = xuiTabsCreate(ctx->pContext, &pTabs, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pTabs, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pTabs, (xui_vec2_t){400.0f, 250.0f});
	xuiWidgetAddChild(ctx->pRoot, pTabs);

	/* 添加标签页 */
	xuiTabsAddPage(pTabs, "Tab 1", &pPage);
	xuiTabsAddPage(pTabs, "Tab 2", NULL);
	xuiTabsAddPage(pTabs, "Tab 3", NULL);

	memset(&ldesc, 0, sizeof(ldesc));
	ldesc.iSize = sizeof(ldesc);
	ldesc.sText = "Tab page content";
	ldesc.pFont = ctx->pFont;
	ldesc.iTextColor = XUI_COLOR_RGBA(50, 60, 80, 255);
	ret = xuiLabelCreate(ctx->pContext, &pLabel, &ldesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pLabel, (xui_thickness_t){12.0f, 12.0f, 0.0f, 0.0f});
	return xuiWidgetAddChild(pPage, pLabel);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch151", argc, argv); }
