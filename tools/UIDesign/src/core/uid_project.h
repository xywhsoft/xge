#ifndef UID_PROJECT_H
#define UID_PROJECT_H

#include "xge.h"

#define UID_PROJECT_NAME_CAPACITY 128
#define UID_PROJECT_PATH_CAPACITY 512

typedef struct uid_project_t {
	char sName[UID_PROJECT_NAME_CAPACITY];
	char sRootPath[UID_PROJECT_PATH_CAPACITY];
	int iPreviewWidth;
	int iPreviewHeight;
	float fPreviewDipScale;
} uid_project_t;

void UIDesignProjectInit(uid_project_t* pProject);
void UIDesignProjectUnit(uid_project_t* pProject);

#endif
