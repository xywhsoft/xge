#ifndef XGE_XUI_INTERNAL_H
#define XGE_XUI_INTERNAL_H

/*
 * Private XUI implementation header.
 *
 * XGE currently builds as a single translation unit, so private helpers with
 * internal linkage are shared by include order. This header marks the internal
 * boundary and is the home for small shared private definitions as XUI controls
 * are split into per-control source files.
 */

#define XGE_XUI_INTERNAL 1

static int __xgeXuiRectContains(xge_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) && (fY >= tRect.fY) && (fX < (tRect.fX + tRect.fW)) && (fY < (tRect.fY + tRect.fH));
}

static float __xgeXuiClampFloat(float fValue, float fMin, float fMax)
{
	if ( fValue < fMin ) {
		return fMin;
	}
	if ( fValue > fMax ) {
		return fMax;
	}
	return fValue;
}

static void __xgeXuiWidgetApplyRolePolicy(xge_xui_widget pWidget, int iRole, int bFocusable, int iImeMode)
{
	if ( pWidget == NULL ) {
		return;
	}
	xgeXuiWidgetSetRole(pWidget, iRole);
	xgeXuiWidgetSetFocusable(pWidget, bFocusable);
	xgeXuiWidgetSetImeMode(pWidget, iImeMode);
}

static void __xgeXuiControlWidgetInit(xge_xui_widget pWidget, int bFocusable)
{
	__xgeXuiWidgetApplyRolePolicy(pWidget, XGE_XUI_WIDGET_ROLE_CONTROL, bFocusable, XGE_XUI_IME_DISABLED);
}

static void __xgeXuiTextControlWidgetInit(xge_xui_widget pWidget)
{
	__xgeXuiWidgetApplyRolePolicy(pWidget, XGE_XUI_WIDGET_ROLE_CONTROL, 1, XGE_XUI_IME_ENABLED);
}

static void __xgeXuiViewportWidgetInit(xge_xui_widget pWidget, int bFocusable)
{
	__xgeXuiWidgetApplyRolePolicy(pWidget, XGE_XUI_WIDGET_ROLE_VIEWPORT, bFocusable, XGE_XUI_IME_DISABLED);
}

static void __xgeXuiOverlayWidgetInit(xge_xui_widget pWidget, int bFocusable)
{
	__xgeXuiWidgetApplyRolePolicy(pWidget, XGE_XUI_WIDGET_ROLE_OVERLAY, bFocusable, XGE_XUI_IME_DISABLED);
}

#endif
