#ifndef UID_DOCUMENT_H
#define UID_DOCUMENT_H

#include "xge.h"

#define UID_DOCUMENT_NAME_CAPACITY 128
#define UID_DOCUMENT_PATH_CAPACITY 512

typedef struct uid_document_t {
	char sName[UID_DOCUMENT_NAME_CAPACITY];
	char sPath[UID_DOCUMENT_PATH_CAPACITY];
	uint32_t iRevision;
	uint32_t iCleanRevision;
	int bDirty;
} uid_document_t;

void UIDesignDocumentInit(uid_document_t* pDocument);
void UIDesignDocumentUnit(uid_document_t* pDocument);
void UIDesignDocumentMarkDirty(uid_document_t* pDocument);

#endif
