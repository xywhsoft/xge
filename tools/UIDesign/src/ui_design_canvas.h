#ifndef UI_DESIGN_CANVAS_H
#define UI_DESIGN_CANVAS_H

#include "ui_design_app.h"

#ifdef __cplusplus
extern "C" {
#endif

int uiDesignCanvasCreate(ui_design_app_t* pApp);
int uiDesignCanvasWorldToDesign(ui_design_app_t* pApp, float fWorldX, float fWorldY, float* pDesignX, float* pDesignY);
int uiDesignCanvasDropTool(ui_design_app_t* pApp, ui_design_node_type_t iType, float fWorldX, float fWorldY);
int uiDesignCanvasPlaceToolRect(ui_design_app_t* pApp, ui_design_node_type_t iType, xui_rect_t tDesignRect, int* pId);
int uiDesignCanvasSelectRect(ui_design_app_t* pApp, xui_rect_t tDesignRect);

#ifdef __cplusplus
}
#endif

#endif
