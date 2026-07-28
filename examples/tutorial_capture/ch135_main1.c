/* ch135 — Radio 单选按钮 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_radio_group_desc_t gdesc;
	xui_radio_desc_t desc;
	xui_widget pGroup, pRadio;
	int ret;

	/* 创建单选组 */
	memset(&gdesc, 0, sizeof(gdesc));
	gdesc.iSize = sizeof(gdesc);
	ret = xuiRadioGroupCreate(ctx->pContext, &pGroup, &gdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetLayoutType(pGroup, XUI_LAYOUT_COLUMN);
	xuiWidgetSetSizeMode(pGroup, XUI_SIZE_CONTENT, XUI_SIZE_CONTENT);
	xuiWidgetSetMargin(pGroup, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetAddChild(ctx->pRoot, pGroup);

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;

	desc.sText = "Option A";
	desc.bChecked = 1;
	ret = xuiRadioCreate(ctx->pContext, &pRadio, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pRadio, (xui_thickness_t){0.0f, 0.0f, 0.0f, 8.0f});
	xuiWidgetAddChild(pGroup, pRadio);

	desc.sText = "Option B";
	desc.bChecked = 0;
	ret = xuiRadioCreate(ctx->pContext, &pRadio, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pRadio, (xui_thickness_t){0.0f, 0.0f, 0.0f, 8.0f});
	xuiWidgetAddChild(pGroup, pRadio);

	desc.sText = "Option C";
	ret = xuiRadioCreate(ctx->pContext, &pRadio, &desc);
	if (ret != XUI_OK) return ret;
	return xuiWidgetAddChild(pGroup, pRadio);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch135", argc, argv); }
