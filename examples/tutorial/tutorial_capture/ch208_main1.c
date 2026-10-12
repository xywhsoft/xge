/* ch208 — 实战：聊天界面 */
#include "tut_capture_xui.h"

static xui_message_node_t s_msgs[] = {
	{ sizeof(xui_message_node_t), "m1", "Alice", "09:30", "Good morning!", 0, 0, NULL, NULL, NULL, 0 },
	{ sizeof(xui_message_node_t), "m2", "Bob",   "09:31", "Morning! Ready for the meeting?", 1, 0, NULL, NULL, NULL, 0 },
	{ sizeof(xui_message_node_t), "m3", "Alice", "09:32", "Yes, let's start at 10.", 0, 0, NULL, NULL, NULL, 0 },
	{ sizeof(xui_message_node_t), "m4", "Bob",   "09:33", "Sounds good!", 1, 0, NULL, NULL, NULL, 0 },
};

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_message_list_desc_t mdesc;
	xui_input_desc_t idesc;
	xui_button_desc_t bdesc;
	xui_widget pMsgList, pInput, pBtn;
	int ret;

	/* 消息列表 */
	memset(&mdesc, 0, sizeof(mdesc));
	mdesc.iSize = sizeof(mdesc);
	mdesc.arrNodes = s_msgs;
	mdesc.iNodeCount = 4;
	mdesc.pFont = ctx->pFont;
	mdesc.bAutoScroll = 1;
	ret = xuiMessageListCreate(ctx->pContext, &pMsgList, &mdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetSizeMode(pMsgList, XUI_SIZE_FILL, XUI_SIZE_FILL);
	xuiWidgetAddChild(ctx->pRoot, pMsgList);

	/* 输入框 */
	memset(&idesc, 0, sizeof(idesc));
	idesc.iSize = sizeof(idesc);
	idesc.pFont = ctx->pFont;
	idesc.sPlaceholder = "Type a message...";
	ret = xuiInputCreate(ctx->pContext, &pInput, &idesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetPreferredSize(pInput, (xui_vec2_t){0.0f, 36.0f});
	xuiWidgetSetSizeMode(pInput, XUI_SIZE_FILL, XUI_SIZE_FIXED);
	xuiWidgetAddChild(ctx->pRoot, pInput);

	/* 发送按钮 */
	memset(&bdesc, 0, sizeof(bdesc));
	bdesc.iSize = sizeof(bdesc);
	bdesc.pFont = ctx->pFont;
	bdesc.sText = "Send";
	ret = xuiButtonCreate(ctx->pContext, &pBtn, &bdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetPreferredSize(pBtn, (xui_vec2_t){80.0f, 36.0f});
	return xuiWidgetAddChild(ctx->pRoot, pBtn);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch208", argc, argv); }
