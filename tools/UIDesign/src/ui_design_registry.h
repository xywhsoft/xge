#ifndef UI_DESIGN_REGISTRY_H
#define UI_DESIGN_REGISTRY_H

#include "ui_design_model.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UI_DESIGN_CONTROL_CONTAINER 0x00000001u

#define UI_DESIGN_PROPERTY_INLINE 0
#define UI_DESIGN_PROPERTY_COMPLEX 1

struct ui_design_app_t;

typedef struct ui_design_property_def_t {
	const char* sId;
	const char* sName;
	const char* sCategory;
	const char* sDescription;
	const char* sDefaultValue;
	int iType;
	int iFlags;
	int iEditorKind;
	const xui_combobox_item_t* pEnumItems;
	int iEnumCount;
} ui_design_property_def_t;

typedef struct ui_design_control_desc_t {
	ui_design_node_type_t iType;
	const char* sTypeName;
	const char* sTitle;
	const char* sCategory;
	uint32_t iFlags;
	float fDefaultW;
	float fDefaultH;
	const ui_design_property_def_t* pProperties;
	int iPropertyCount;
	int (*onCreate)(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
	int (*onApply)(struct ui_design_app_t* pApp, ui_design_node_t* pNode);
} ui_design_control_desc_t;

int uiDesignRegistryGetCount(void);
const ui_design_control_desc_t* uiDesignRegistryGetAt(int iIndex);
const ui_design_control_desc_t* uiDesignRegistryFind(ui_design_node_type_t iType);
int uiDesignRegistryGetCategoryIndex(const char* sCategory);
int uiDesignRegistryGetCategoryCount(void);
const char* uiDesignRegistryGetCategory(int iIndex);
const ui_design_property_def_t* uiDesignRegistryFindProperty(const ui_design_control_desc_t* pDesc, const char* sId);
int uiDesignRegistryInitNodeProperties(const ui_design_control_desc_t* pDesc, ui_design_node_t* pNode);

#ifdef __cplusplus
}
#endif

#endif
