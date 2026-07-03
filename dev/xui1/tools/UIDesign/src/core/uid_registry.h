#ifndef UID_REGISTRY_H
#define UID_REGISTRY_H

#include "xge.h"

#define UID_CONTROL_REGISTRY_CAPACITY 128

typedef enum uid_control_category_t {
	UID_CONTROL_CATEGORY_BASIC = 0,
	UID_CONTROL_CATEGORY_INPUT,
	UID_CONTROL_CATEGORY_LAYOUT,
	UID_CONTROL_CATEGORY_COLLECTION,
	UID_CONTROL_CATEGORY_DATA,
	UID_CONTROL_CATEGORY_OVERLAY,
	UID_CONTROL_CATEGORY_WORKBENCH
} uid_control_category_t;

typedef struct uid_control_editor_desc_t {
	const char* sType;
	const char* sDisplayName;
	int iCategory;
	uint32_t iCapabilities;
} uid_control_editor_desc_t;

typedef struct uid_control_registry_t {
	uid_control_editor_desc_t arrItems[UID_CONTROL_REGISTRY_CAPACITY];
	int iCount;
} uid_control_registry_t;

void UIDesignControlRegistryInit(uid_control_registry_t* pRegistry);
void UIDesignControlRegistryUnit(uid_control_registry_t* pRegistry);
int UIDesignControlRegistryAdd(uid_control_registry_t* pRegistry, const uid_control_editor_desc_t* pDesc);
int UIDesignControlRegistryRegisterBuiltins(uid_control_registry_t* pRegistry);

#endif
