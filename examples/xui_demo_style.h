#ifndef XGE_EXAMPLES_XUI_DEMO_STYLE_H
#define XGE_EXAMPLES_XUI_DEMO_STYLE_H

#define XGE_XUI_DEMO_FONT_SIZE 12.0f

static inline void XgeXuiDemoTheme(xge_xui_theme_t* pTheme, xge_font pFont)
{
	xgeXuiThemeDefault(pTheme);
	pTheme->pFont = pFont;
	pTheme->iTextColor = XGE_COLOR_RGBA(36, 42, 52, 255);
	pTheme->iBackgroundColor = XGE_COLOR_RGBA(236, 240, 246, 255);
	pTheme->iPanelColor = XGE_COLOR_RGBA(248, 250, 253, 255);
	pTheme->iBorderColor = XGE_COLOR_RGBA(162, 174, 190, 255);
	pTheme->iAccentColor = XGE_COLOR_RGBA(46, 124, 214, 255);
	pTheme->iSelectionColor = XGE_COLOR_RGBA(46, 124, 214, 96);
	pTheme->iStateNormal = XGE_COLOR_RGBA(232, 238, 247, 255);
	pTheme->iStateHover = XGE_COLOR_RGBA(218, 230, 246, 255);
	pTheme->iStateActive = XGE_COLOR_RGBA(198, 216, 240, 255);
	pTheme->iStateFocus = XGE_COLOR_RGBA(126, 166, 220, 255);
	pTheme->iStateDisabled = XGE_COLOR_RGBA(206, 211, 218, 170);
	pTheme->fRadius = 4.0f;
	pTheme->fPadding = 6.0f;
	pTheme->fSpacing = 8.0f;
	pTheme->fBorderWidth = 1.0f;
}

static inline void XgeXuiDemoApplyTheme(xge_xui_context pContext, xge_font pFont)
{
	xge_xui_theme_t tTheme;

	XgeXuiDemoTheme(&tTheme, pFont);
	xgeXuiSetTheme(pContext, &tTheme);
}

static inline void XgeXuiDemoApplyRootPanel(xge_xui_widget pWidget)
{
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(236, 240, 246, 255));
	xgeXuiWidgetSetRadius(pWidget, 0.0f);
}

static inline void XgeXuiDemoApplyPanel(xge_xui_widget pWidget)
{
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(248, 250, 253, 255));
	xgeXuiWidgetSetRadius(pWidget, 4.0f);
}

static inline void XgeXuiDemoApplyStatus(xge_xui_widget pWidget)
{
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(224, 232, 242, 255));
	xgeXuiWidgetSetRadius(pWidget, 0.0f);
	xgeXuiWidgetSetPaddingPx(pWidget, 8.0f, 5.0f, 8.0f, 5.0f);
}

#endif
