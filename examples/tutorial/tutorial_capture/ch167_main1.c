/* ch167 — DatePicker 日期选择 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_date_picker_desc_t desc;
	xui_widget pPicker;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.iMode = 0; /* date only */
	desc.bHasValue = 1;
	desc.tValue = 1717200000; /* 2024-06-01 approx */
	desc.pFont = ctx->pFont;
	desc.sFormat = "yyyy-MM-dd";
	ret = xuiDatePickerCreate(ctx->pContext, &pPicker, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pPicker, (xui_thickness_t){40.0f, 40.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pPicker, (xui_vec2_t){200.0f, 36.0f});
	return xuiWidgetAddChild(ctx->pRoot, pPicker);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch167", argc, argv); }
