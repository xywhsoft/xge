#include "milestone_view.h"

#include <stdio.h>
#include <string.h>

static int g_iSurfaceResolveCount;
static int g_iActionCount;
static xui_widget g_pActionWidget;
static int g_iActionUser;

static void __testAction(const char* sAction, xui_widget pWidget, void* pUser)
{
	if ( sAction != NULL && strcmp(sAction, "settings.apply") == 0 &&
	     pUser == &g_iActionUser ) {
		g_iActionCount++;
		g_pActionWidget = pWidget;
	}
}

static xui_surface __testResolveSurface(const char* sPath, void* pUser)
{
	(void)pUser;
	if ( sPath != NULL && strcmp(sPath, "xui_image_file.png") == 0 ) g_iSurfaceResolveCount++;
	return NULL;
}

int main(void)
{
	milestone_view_callbacks_t tCallbacks;
	milestone_view_view_t tView;
	xui_context pContext;
	xui_widget pPanelClient;
	xui_widget pTabPage;
	int iRet;

	pContext = NULL;
	iRet = xuiCreate(&pContext);
	if ( iRet != XUI_OK || pContext == NULL ) return 1;
	memset(&tCallbacks, 0, sizeof(tCallbacks));
	tCallbacks.onAction = __testAction;
	tCallbacks.resolveSurface = __testResolveSurface;
	tCallbacks.pUser = &g_iActionUser;
	memset(&tView, 0, sizeof(tView));
	iRet = milestone_view_create(pContext, NULL, NULL, &tCallbacks, &tView);
	if ( iRet != XUI_OK ) {
		xuiDestroy(pContext);
		return 2;
	}
	if ( tView.pRoot != tView.widget_1 || tView.panel_2 == NULL || tView.label_3 == NULL ||
	     tView.button_4 == NULL || tView.input_5 == NULL || tView.checkbox_6 == NULL ||
	     tView.tabs_7 == NULL || tView.image_8 == NULL || g_iSurfaceResolveCount != 1 ) {
		milestone_view_destroy(&tView);
		xuiDestroy(pContext);
		return 3;
	}
	pPanelClient = xuiPanelGetClientWidget(tView.panel_2);
	pTabPage = xuiTabsGetPageWidget(tView.tabs_7, 1);
	if ( xuiWidgetGetParent(tView.panel_2) != tView.widget_1 ||
	     xuiWidgetGetParent(tView.label_3) != pPanelClient ||
	     xuiWidgetGetParent(tView.button_4) != pPanelClient ||
	     xuiWidgetGetParent(tView.input_5) != pPanelClient ||
	     xuiWidgetGetParent(tView.checkbox_6) != pPanelClient ||
	     xuiWidgetGetParent(tView.tabs_7) != tView.widget_1 ||
	     pTabPage == NULL || xuiWidgetGetParent(tView.image_8) != pTabPage ) {
		milestone_view_destroy(&tView);
		xuiDestroy(pContext);
		return 4;
	}
	/* A host attaches the generated root before sending interactive input. */
	if ( xuiSetRootWidget(pContext, tView.pRoot) != XUI_OK ||
	     xuiSetViewportSize(pContext, 900, 600) != XUI_OK || xuiLayout(pContext) != XUI_OK ) return 7;
	if ( xuiSetFocusWidget(pContext, tView.button_4) != XUI_OK ||
	     xuiInputKeyDown(pContext, XUI_KEY_SPACE, 0u) != XUI_OK ||
	     xuiDispatchPendingEvents(pContext) != XUI_OK ||
	     xuiInputKeyUp(pContext, XUI_KEY_SPACE, 0u) != XUI_OK ||
	     xuiDispatchPendingEvents(pContext) != XUI_OK ||
	     g_iActionCount != 1 || g_pActionWidget != tView.button_4 ||
	     xuiButtonGetClickCount(tView.button_4) != 1 ) {
		fprintf(stderr, "generated-view action failed: focus=%d actions=%d clicks=%d\n",
			xuiGetFocusWidget(pContext) == tView.button_4, g_iActionCount, xuiButtonGetClickCount(tView.button_4));
		milestone_view_destroy(&tView);
		xuiDestroy(pContext);
		return 5;
	}
	milestone_view_destroy(&tView);
	if ( tView.pRoot != NULL || tView.widget_1 != NULL || tView.image_8 != NULL ) {
		xuiDestroy(pContext);
		return 6;
	}
	xuiDestroy(pContext);
	printf("uidesign-generated-view-test passed\n");
	return 0;
}
