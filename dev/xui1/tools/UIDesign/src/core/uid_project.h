#ifndef UID_PROJECT_H
#define UID_PROJECT_H

#include "xge.h"

#define UID_PROJECT_NAME_CAPACITY 128
#define UID_PROJECT_PATH_CAPACITY 512

typedef struct uid_document_t uid_document_t;

typedef struct uid_project_t {
	char sName[UID_PROJECT_NAME_CAPACITY];
	char sRootPath[UID_PROJECT_PATH_CAPACITY];
	char sCurrentPage[UID_PROJECT_PATH_CAPACITY];
	int iPreviewWidth;
	int iPreviewHeight;
	float fPreviewDipScale;
	int bReady;
} uid_project_t;

void UIDesignProjectInit(uid_project_t* pProject);
void UIDesignProjectUnit(uid_project_t* pProject);
int UIDesignProjectEnsureDefault(uid_project_t* pProject);
int UIDesignProjectSaveDocument(uid_project_t* pProject, const uid_document_t* pDocument);

#endif
