#include "../audit_xui_common.h"

static audit_xui_ctx_t g_ctx;

static int create_ui(audit_xui_ctx_t* ctx)
{
	xui_widget w;
	xui_button_desc_t btnDesc;
	xui_checkbox_desc_t chkDesc;
	xui_radio_desc_t radioDesc;
	xui_toggle_desc_t togDesc;
	xui_slider_desc_t sldDesc;
	xui_progress_desc_t prgDesc;
	xui_label_desc_t lblDesc;

	/* Row 1: Buttons */
	memset(&lblDesc, 0, sizeof(lblDesc)); lblDesc.iSize = sizeof(lblDesc); lblDesc.sText = "Buttons:"; lblDesc.pFont = ctx->pFont;
	xuiLabelCreate(ctx->pContext, &w, &lblDesc); xuiWidgetAddChild(ctx->pRoot, w);
	memset(&btnDesc, 0, sizeof(btnDesc)); btnDesc.iSize = sizeof(btnDesc);
	btnDesc.sText = "Normal"; btnDesc.pFont = ctx->pFont;
	xuiButtonCreate(ctx->pContext, &w, &btnDesc); xuiWidgetAddChild(ctx->pRoot, w);
	btnDesc.sText = "Disabled";
	xuiButtonCreate(ctx->pContext, &w, &btnDesc); xuiWidgetSetEnabled(w, 0); xuiWidgetAddChild(ctx->pRoot, w);
	btnDesc.sText = "Checked"; btnDesc.iCheckedColor = XUI_COLOR_RGBA(80,180,80,255);
	xuiButtonCreate(ctx->pContext, &w, &btnDesc); xuiWidgetAddChild(ctx->pRoot, w);

	/* Row 2: Checkboxes */
	memset(&lblDesc, 0, sizeof(lblDesc)); lblDesc.iSize = sizeof(lblDesc); lblDesc.sText = "Checkboxes:"; lblDesc.pFont = ctx->pFont;
	xuiLabelCreate(ctx->pContext, &w, &lblDesc); xuiWidgetAddChild(ctx->pRoot, w);
	memset(&chkDesc, 0, sizeof(chkDesc)); chkDesc.iSize = sizeof(chkDesc);
	chkDesc.sText = "Unchecked"; chkDesc.pFont = ctx->pFont; chkDesc.bUseBuiltinAtlas = 1;
	xuiCheckBoxCreate(ctx->pContext, &w, &chkDesc); xuiWidgetAddChild(ctx->pRoot, w);
	chkDesc.sText = "Checked"; chkDesc.bChecked = 1;
	xuiCheckBoxCreate(ctx->pContext, &w, &chkDesc); xuiWidgetAddChild(ctx->pRoot, w);
	chkDesc.sText = "Disabled"; chkDesc.bChecked = 0;
	xuiCheckBoxCreate(ctx->pContext, &w, &chkDesc); xuiWidgetSetEnabled(w, 0); xuiWidgetAddChild(ctx->pRoot, w);

	/* Row 3: Radios */
	memset(&lblDesc, 0, sizeof(lblDesc)); lblDesc.iSize = sizeof(lblDesc); lblDesc.sText = "Radios:"; lblDesc.pFont = ctx->pFont;
	xuiLabelCreate(ctx->pContext, &w, &lblDesc); xuiWidgetAddChild(ctx->pRoot, w);
	memset(&radioDesc, 0, sizeof(radioDesc)); radioDesc.iSize = sizeof(radioDesc);
	radioDesc.sText = "Option A"; radioDesc.pFont = ctx->pFont; radioDesc.bUseBuiltinAtlas = 1;
	xuiRadioCreate(ctx->pContext, &w, &radioDesc); xuiWidgetAddChild(ctx->pRoot, w);
	radioDesc.sText = "Option B (sel)"; radioDesc.bChecked = 1;
	xuiRadioCreate(ctx->pContext, &w, &radioDesc); xuiWidgetAddChild(ctx->pRoot, w);
	radioDesc.sText = "Disabled"; radioDesc.bChecked = 0;
	xuiRadioCreate(ctx->pContext, &w, &radioDesc); xuiWidgetSetEnabled(w, 0); xuiWidgetAddChild(ctx->pRoot, w);

	/* Row 4: Toggles */
	memset(&lblDesc, 0, sizeof(lblDesc)); lblDesc.iSize = sizeof(lblDesc); lblDesc.sText = "Toggles:"; lblDesc.pFont = ctx->pFont;
	xuiLabelCreate(ctx->pContext, &w, &lblDesc); xuiWidgetAddChild(ctx->pRoot, w);
	memset(&togDesc, 0, sizeof(togDesc)); togDesc.iSize = sizeof(togDesc);
	togDesc.sText = "Off"; togDesc.pFont = ctx->pFont;
	xuiToggleCreate(ctx->pContext, &w, &togDesc); xuiWidgetAddChild(ctx->pRoot, w);
	togDesc.sText = "On"; togDesc.bChecked = 1;
	xuiToggleCreate(ctx->pContext, &w, &togDesc); xuiWidgetAddChild(ctx->pRoot, w);
	togDesc.sText = "Disabled"; togDesc.bChecked = 0;
	xuiToggleCreate(ctx->pContext, &w, &togDesc); xuiWidgetSetEnabled(w, 0); xuiWidgetAddChild(ctx->pRoot, w);

	/* Row 5: Sliders */
	memset(&lblDesc, 0, sizeof(lblDesc)); lblDesc.iSize = sizeof(lblDesc); lblDesc.sText = "Sliders:"; lblDesc.pFont = ctx->pFont;
	xuiLabelCreate(ctx->pContext, &w, &lblDesc); xuiWidgetAddChild(ctx->pRoot, w);
	memset(&sldDesc, 0, sizeof(sldDesc)); sldDesc.iSize = sizeof(sldDesc);
	sldDesc.fMin = 0; sldDesc.fMax = 100; sldDesc.fValue = 30;
	xuiSliderCreate(ctx->pContext, &w, &sldDesc); xuiWidgetAddChild(ctx->pRoot, w);
	sldDesc.fValue = 70;
	xuiSliderCreate(ctx->pContext, &w, &sldDesc); xuiWidgetAddChild(ctx->pRoot, w);
	sldDesc.fValue = 50;
	xuiSliderCreate(ctx->pContext, &w, &sldDesc); xuiWidgetSetEnabled(w, 0); xuiWidgetAddChild(ctx->pRoot, w);

	/* Row 6: Progress bars */
	memset(&lblDesc, 0, sizeof(lblDesc)); lblDesc.iSize = sizeof(lblDesc); lblDesc.sText = "Progress:"; lblDesc.pFont = ctx->pFont;
	xuiLabelCreate(ctx->pContext, &w, &lblDesc); xuiWidgetAddChild(ctx->pRoot, w);
	memset(&prgDesc, 0, sizeof(prgDesc)); prgDesc.iSize = sizeof(prgDesc);
	prgDesc.fMin = 0; prgDesc.fMax = 100; prgDesc.fValue = 25;
	xuiProgressCreate(ctx->pContext, &w, &prgDesc); xuiWidgetAddChild(ctx->pRoot, w);
	prgDesc.fValue = 60;
	xuiProgressCreate(ctx->pContext, &w, &prgDesc); xuiWidgetAddChild(ctx->pRoot, w);
	prgDesc.fValue = 100;
	xuiProgressCreate(ctx->pContext, &w, &prgDesc); xuiWidgetAddChild(ctx->pRoot, w);

	return XUI_OK;
}

static int frame(void* user)
{
	return audit_xui_frame((audit_xui_ctx_t*)user);
}

int main(int argc, char** argv)
{
	int ret;
	setbuf(stdout, NULL);
	memset(&g_ctx, 0, sizeof(g_ctx));
	audit_xui_parse(&g_ctx, argc, argv);
	ret = audit_xui_init(&g_ctx, "audit_xui_basic");
	if (ret != XGE_OK) { printf("init failed: %d\n", ret); return 1; }
	ret = create_ui(&g_ctx);
	if (ret != XUI_OK) { printf("create_ui failed: %d\n", ret); audit_xui_shutdown(&g_ctx); return 1; }
	ret = xgeRun(frame, &g_ctx);
	audit_xui_shutdown(&g_ctx);
	return (ret == XGE_OK) ? 0 : 1;
}
