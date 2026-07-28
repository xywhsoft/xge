/* ch185 — MessageList 消息列表 */
#include "tut_capture_xui.h"

static xui_message_node_t s_msgs[] = {
	{ sizeof(xui_message_node_t), "m1", "Alice", "10:01", "Hello!", 0, 0, NULL, NULL, NULL, 0 },
	{ sizeof(xui_message_node_t), "m2", "Bob",   "10:02", "Hi Alice!", 1, 0, NULL, NULL, NULL, 0 },
	{ sizeof(xui_message_node_t), "m3", "Alice", "10:03", "How are you?", 0, 0, NULL, NULL, NULL, 0 },
};

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_message_list_desc_t desc;
	xui_widget pList;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.arrNodes = s_msgs;
	desc.iNodeCount = 3;
	desc.pFont = ctx->pFont;
	desc.bAutoScroll = 1;
	ret = xuiMessageListCreate(ctx->pContext, &pList, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pList, (xui_thickness_t){20.0f, 20.0f, 20.0f, 20.0f});
	xuiWidgetSetSizeMode(pList, XUI_SIZE_FILL, XUI_SIZE_FILL);
	return xuiWidgetAddChild(ctx->pRoot, pList);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch185", argc, argv); }
