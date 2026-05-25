#include "tileset_materials_preview.h"

int MapEditTilesetMaterialsPreviewCreate(mapedit_app_t* pApp, xge_xui_dock_window pWindow)
{
	xge_xui_widget pContent;

	pContent = MapEditWorkspaceNewContent(pApp);
	if ( pContent == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pApp->pTileSourcePreviewWidget = pContent;
	xgeXuiDockWindowSetClientWidget(pWindow, pContent);
	return XGE_OK;
}
