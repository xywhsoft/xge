/* ch194 — 国际化 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_label_desc_t desc;
	xui_widget pLabel;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.pFont = ctx->pFont;
	desc.sText = "English: Hello World\nChinese: \xE4\xBD\xA0\xE5\xA5\xBD\xE4\xB8\x96\xE7\x95\x8C\nJapanese: \xE3\x81\x93\xE3\x82\x93\xE3\x81\xAB\xE3\x81\xA1\xE3\x81\xAF";
	ret = xuiLabelCreate(ctx->pContext, &pLabel, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pLabel, (xui_thickness_t){30.0f, 30.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pLabel, (xui_vec2_t){400.0f, 100.0f});
	return xuiWidgetAddChild(ctx->pRoot, pLabel);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch194", argc, argv); }
