#ifndef UI_DESIGN_CODEGEN_H
#define UI_DESIGN_CODEGEN_H

#include "ui_design_model.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Export the supported runtime subset (Widget, Panel, Label, Button, Input,
 * CheckBox, Image and Tabs) as a self-contained C/header pair.  sCPath is the
 * requested .c path; the sibling .h path is returned when a buffer is given.
 */
int uiDesignCodegenExport(const ui_design_model_t* pModel, const char* sCPath,
	const char* sSymbolPrefix, char* sHeaderPath, int iHeaderPathCapacity);
int uiDesignCodegenSupportsControl(ui_design_node_type_t iType);

#ifdef __cplusplus
}
#endif

#endif
