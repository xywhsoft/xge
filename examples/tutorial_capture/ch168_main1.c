/* ch168 — TagInput 标签输入 */
#include "tut_capture_xui.h"

static const char* s_tags[] = { "C", "C++", "Rust" };

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_tag_input_desc_t desc;
	xui_widget pTagInput;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.ppTags = s_tags;
	desc.iTagCount = 3;
	desc.sPlaceholder = "Add language...";
	desc.pFont = ctx->pFont;
	desc.iMaxTags = 10;
	ret = xuiTagInputCreate(ctx->pContext, &pTagInput, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pTagInput, (xui_thickness_t){40.0f, 40.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pTagInput, (xui_vec2_t){360.0f, 40.0f});
	return xuiWidgetAddChild(ctx->pRoot, pTagInput);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch168", argc, argv); }
