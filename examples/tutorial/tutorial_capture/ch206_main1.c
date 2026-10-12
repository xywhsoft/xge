/* ch206 — 实战：游戏 HUD */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_progress_desc_t pdesc;
	xui_label_desc_t ldesc;
	xui_virtual_joystick_desc_t jdesc;
	xui_widget pHP, pMP, pLabel, pJoy;
	int ret;

	/* HP 条 */
	memset(&pdesc, 0, sizeof(pdesc));
	pdesc.iSize = sizeof(pdesc);
	pdesc.pFont = ctx->pFont;
	pdesc.fMin = 0.0f;
	pdesc.fMax = 100.0f;
	pdesc.fValue = 80.0f;
	pdesc.sText = "HP 80/100";
	ret = xuiProgressCreate(ctx->pContext, &pHP, &pdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetRect(pHP, (xui_rect_t){20.0f, 20.0f, 250.0f, 22.0f});
	xuiWidgetAddChild(ctx->pRoot, pHP);

	/* MP 条 */
	pdesc.fValue = 55.0f;
	pdesc.sText = "MP 55/100";
	ret = xuiProgressCreate(ctx->pContext, &pMP, &pdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetRect(pMP, (xui_rect_t){20.0f, 48.0f, 250.0f, 22.0f});
	xuiWidgetAddChild(ctx->pRoot, pMP);

	/* 分数标签 */
	memset(&ldesc, 0, sizeof(ldesc));
	ldesc.iSize = sizeof(ldesc);
	ldesc.pFont = ctx->pFont;
	ldesc.sText = "Score: 12450";
	ret = xuiLabelCreate(ctx->pContext, &pLabel, &ldesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetRect(pLabel, (xui_rect_t){700.0f, 20.0f, 150.0f, 30.0f});
	xuiWidgetAddChild(ctx->pRoot, pLabel);

	/* 虚拟摇杆 */
	memset(&jdesc, 0, sizeof(jdesc));
	jdesc.iSize = sizeof(jdesc);
	jdesc.fRadius = 60.0f;
	jdesc.fKnobSize = 28.0f;
	jdesc.bUseBuiltinAtlas = 1;
	ret = xuiVirtualJoystickCreate(ctx->pContext, &pJoy, &jdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetRect(pJoy, (xui_rect_t){60.0f, 380.0f, 140.0f, 140.0f});
	return xuiWidgetAddChild(ctx->pRoot, pJoy);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch206", argc, argv); }
