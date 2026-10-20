#ifndef UI_DESIGN_INSPECTOR_H
#define UI_DESIGN_INSPECTOR_H

#include "ui_design_app.h"

#ifdef __cplusplus
extern "C" {
#endif

int uiDesignInspectorCreate(ui_design_app_t* pApp);
int uiDesignInspectorRefresh(ui_design_app_t* pApp);
void uiDesignInspectorUpdateComplexEditor(ui_design_app_t* pApp);
void uiDesignInspectorCloseComplexEditor(ui_design_app_t* pApp);
int uiDesignInspectorShowCollectionDemo(ui_design_app_t* pApp);
int uiDesignInspectorExerciseComplexEditors(ui_design_app_t* pApp);
int uiDesignInspectorExerciseCollectionEditors(ui_design_app_t* pApp);
int uiDesignInspectorExerciseCollectionCatalog(ui_design_app_t* pApp);
int uiDesignInspectorExercisePropertyCoverage(ui_design_app_t* pApp);

#ifdef __cplusplus
}
#endif

#endif
