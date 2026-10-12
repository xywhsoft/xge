#ifndef UI_DESIGN_WORKBENCH_H
#define UI_DESIGN_WORKBENCH_H

#include "ui_design_app.h"

int uiDesignWorkbenchLoadFont(ui_design_app_t* pApp);
int uiDesignWorkbenchCreateTabs(ui_design_app_t* pApp);
void uiDesignWorkbenchRefreshTabs(ui_design_app_t* pApp);
int uiDesignWorkbenchInit(ui_design_app_t* pApp);
int uiDesignWorkbenchTick(ui_design_app_t* pApp);
void uiDesignWorkbenchShutdown(ui_design_app_t* pApp);
int uiDesignWorkbenchResetLayout(ui_design_app_t* pApp);
int uiDesignWorkbenchSaveLayout(ui_design_app_t* pApp);
int uiDesignWorkbenchLoadLayout(ui_design_app_t* pApp);
int uiDesignWorkbenchScreenshot(ui_design_app_t* pApp);
void uiDesignAppCancelGesture(ui_design_app_t* pApp);

#endif
