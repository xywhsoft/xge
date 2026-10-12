/* ch164 — ComboBox 组合框 */
#include "tut_capture_xui.h"

static const char* s_items[] = {
	"Apple", "Banana", "Cherry", "Dragon Fruit", "Elderberry"
};

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_combobox_desc_t desc;
	xui_widget pCombo;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.arrItems = s_items;
	desc.iItemCount = 5;
	desc.iSelected = 1;
	desc.pFont = ctx->pFont;
	desc.sPlaceholder = "Select a fruit...";
	ret = xuiComboBoxCreate(ctx->pContext, &pCombo, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pCombo, (xui_thickness_t){40.0f, 40.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pCombo, (xui_vec2_t){220.0f, 36.0f});
	return xuiWidgetAddChild(ctx->pRoot, pCombo);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch164", argc, argv); }
