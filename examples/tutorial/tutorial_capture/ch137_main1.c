/* ch137 — CheckCard 选择卡片 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_check_card_desc_t desc;
	xui_widget pCard;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);

	/* 卡片 1 */
	ret = xuiCheckCardCreate(ctx->pContext, &pCard, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pCard, (xui_thickness_t){20.0f, 20.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pCard, (xui_vec2_t){200.0f, 80.0f});
	xuiWidgetAddChild(ctx->pRoot, pCard);

	/* 卡片 2 */
	ret = xuiCheckCardCreate(ctx->pContext, &pCard, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pCard, (xui_thickness_t){20.0f, 12.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pCard, (xui_vec2_t){200.0f, 80.0f});
	return xuiWidgetAddChild(ctx->pRoot, pCard);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch137", argc, argv); }
