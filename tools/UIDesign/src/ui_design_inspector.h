#ifndef UI_DESIGN_INSPECTOR_H
#define UI_DESIGN_INSPECTOR_H

#include "ui_design_app.h"

#ifdef __cplusplus
extern "C" {
#endif

int uiDesignInspectorCreate(ui_design_app_t* pApp);
int uiDesignInspectorRefresh(ui_design_app_t* pApp);
int uiDesignInspectorExerciseComplexEditors(ui_design_app_t* pApp);

#ifdef __cplusplus
}
#endif

#endif
