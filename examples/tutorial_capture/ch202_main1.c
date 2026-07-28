/* ch202 — 应用骨架：场景 + UI 共存 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_menubar_desc_t mdesc;
	xui_statusbar_desc_t sdesc;
	xui_label_desc_t ldesc;
	xui_widget pMenu, pStatus, pLabel;
	int ret;

	/* 菜单栏 */
	memset(&mdesc, 0, sizeof(mdesc));
	mdesc.iSize = sizeof(mdesc);
	mdesc.pFont = ctx->pFont;
	ret = xuiMenuBarCreate(ctx->pContext, &pMenu, &mdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetSizeMode(pMenu, XUI_SIZE_FILL, XUI_SIZE_CONTENT);
	xuiWidgetAddChild(ctx->pRoot, pMenu);
	xuiMenuBarAddItem(pMenu, "File", NULL, 0);
	xuiMenuBarAddItem(pMenu, "Edit", NULL, 1);
	xuiMenuBarAddItem(pMenu, "View", NULL, 2);

	/* 中心内容 */
	memset(&ldesc, 0, sizeof(ldesc));
	ldesc.iSize = sizeof(ldesc);
	ldesc.pFont = ctx->pFont;
	ldesc.sText = "Scene Viewport";
	ret = xuiLabelCreate(ctx->pContext, &pLabel, &ldesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pLabel, (xui_thickness_t){30.0f, 30.0f, 0.0f, 0.0f});
	xuiWidgetAddChild(ctx->pRoot, pLabel);

	/* 状态栏 */
	memset(&sdesc, 0, sizeof(sdesc));
	sdesc.iSize = sizeof(sdesc);
	sdesc.pFont = ctx->pFont;
	ret = xuiStatusBarCreate(ctx->pContext, &pStatus, &sdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetSizeMode(pStatus, XUI_SIZE_FILL, XUI_SIZE_CONTENT);
	xuiWidgetSetAlign(pStatus, XUI_ALIGN_STRETCH, XUI_ALIGN_END);
	xuiWidgetAddChild(ctx->pRoot, pStatus);
	xuiStatusBarAddText(pStatus, 0, "Ready", 100.0f, 0, 0);
	return XUI_OK;
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch202", argc, argv); }
