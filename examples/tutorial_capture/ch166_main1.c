/* ch166 — ColorPicker 颜色选择器 */
#include "tut_capture_xui.h"

static uint32_t s_palette[] = {
	0xFFFF0000, 0xFF00FF00, 0xFF0000FF, 0xFFFFFF00,
	0xFFFF00FF, 0xFF00FFFF, 0xFF000000, 0xFFFFFFFF
};

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_color_picker_desc_t desc;
	xui_widget pPicker;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.iColor = 0xFF4F9CFF; /* initial color */
	desc.arrPalette = s_palette;
	desc.iPaletteCount = 8;
	desc.bAlphaEnabled = 1;
	desc.pFont = ctx->pFont;
	ret = xuiColorPickerCreate(ctx->pContext, &pPicker, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pPicker, (xui_thickness_t){40.0f, 40.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pPicker, (xui_vec2_t){200.0f, 36.0f});
	return xuiWidgetAddChild(ctx->pRoot, pPicker);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch166", argc, argv); }
