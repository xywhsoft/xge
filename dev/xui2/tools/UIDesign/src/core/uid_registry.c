#include "uid_registry.h"
#include <string.h>

void UIDesignControlRegistryInit(uid_control_registry_t* pRegistry)
{
	if ( pRegistry == NULL ) {
		return;
	}
	memset(pRegistry, 0, sizeof(*pRegistry));
}

void UIDesignControlRegistryUnit(uid_control_registry_t* pRegistry)
{
	(void)pRegistry;
}

int UIDesignControlRegistryAdd(uid_control_registry_t* pRegistry, const uid_control_editor_desc_t* pDesc)
{
	if ( (pRegistry == NULL) || (pDesc == NULL) || (pRegistry->iCount >= UID_CONTROL_REGISTRY_CAPACITY) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pRegistry->arrItems[pRegistry->iCount++] = *pDesc;
	return XGE_OK;
}

int UIDesignControlRegistryRegisterBuiltins(uid_control_registry_t* pRegistry)
{
	static const uid_control_editor_desc_t arrBuiltins[] = {
		{ "label", "Label", UID_CONTROL_CATEGORY_BASIC, 0 },
		{ "button", "Button", UID_CONTROL_CATEGORY_BASIC, 0 },
		{ "input", "Input", UID_CONTROL_CATEGORY_INPUT, 0 },
		{ "image", "Image", UID_CONTROL_CATEGORY_BASIC, 0 },
		{ "panel", "Panel", UID_CONTROL_CATEGORY_LAYOUT, 0 },
		{ "row", "Row", UID_CONTROL_CATEGORY_LAYOUT, 0 },
		{ "column", "Column", UID_CONTROL_CATEGORY_LAYOUT, 0 },
		{ "grid", "Grid", UID_CONTROL_CATEGORY_LAYOUT, 0 },
		{ "scrollView", "ScrollView", UID_CONTROL_CATEGORY_LAYOUT, 0 },
		{ "comboBox", "ComboBox", UID_CONTROL_CATEGORY_COLLECTION, 0 },
		{ "listView", "ListView", UID_CONTROL_CATEGORY_COLLECTION, 0 },
		{ "treeView", "TreeView", UID_CONTROL_CATEGORY_COLLECTION, 0 },
		{ "tabs", "Tabs", UID_CONTROL_CATEGORY_COLLECTION, 0 },
		{ "propertyGrid", "PropertyGrid", UID_CONTROL_CATEGORY_DATA, 0 },
		{ "dockLayout", "DockLayout", UID_CONTROL_CATEGORY_WORKBENCH, 0 }
	};
	int i;

	if ( pRegistry == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pRegistry->iCount = 0;
	for ( i = 0; i < (int)(sizeof(arrBuiltins) / sizeof(arrBuiltins[0])); i++ ) {
		if ( UIDesignControlRegistryAdd(pRegistry, &arrBuiltins[i]) != XGE_OK ) {
			return XGE_ERROR;
		}
	}
	return XGE_OK;
}
