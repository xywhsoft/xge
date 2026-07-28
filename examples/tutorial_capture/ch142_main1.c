/* ch142 — Separator 分隔线 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_label_desc_t ldesc;
	xui_separator_desc_t sdesc;
	xui_widget pLabel, pSep;
	int ret;

	memset(&ldesc, 0, sizeof(ldesc));
	ldesc.iSize = sizeof(ldesc);
	ldesc.pFont = ctx->pFont;
	ldesc.iTextColor = XUI_COLOR_RGBA(30, 40, 60, 255);

	/* 标签 1 */
	ldesc.sText = "Section One";
	ret = xuiLabelCreate(ctx->pContext, &pLabel, &ldesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pLabel, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetAddChild(ctx->pRoot, pLabel);

	/* 水平分隔线 */
	memset(&sdesc, 0, sizeof(sdesc));
	sdesc.iSize = sizeof(sdesc);
	sdesc.iColor = XUI_COLOR_RGBA(180, 190, 210, 255);
	sdesc.fThickness = 1.0f;
	sdesc.iOrientation = 0; /* horizontal */
	ret = xuiSeparatorCreate(ctx->pContext, &pSep, &sdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pSep, (xui_thickness_t){20.0f, 12.0f, 20.0f, 12.0f});
	xuiWidgetSetSizeMode(pSep, XUI_SIZE_FILL, XUI_SIZE_FIXED);
	xuiWidgetAddChild(ctx->pRoot, pSep);

	/* 标签 2 */
	ldesc.sText = "Section Two";
	ret = xuiLabelCreate(ctx->pContext, &pLabel, &ldesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetAddChild(ctx->pRoot, pLabel);

	/* 粗分隔线 */
	sdesc.fThickness = 3.0f;
	sdesc.iColor = XUI_COLOR_RGBA(100, 140, 200, 255);
	ret = xuiSeparatorCreate(ctx->pContext, &pSep, &sdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pSep, (xui_thickness_t){20.0f, 12.0f, 20.0f, 12.0f});
	xuiWidgetSetSizeMode(pSep, XUI_SIZE_FILL, XUI_SIZE_FIXED);
	return xuiWidgetAddChild(ctx->pRoot, pSep);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch142", argc, argv); }
