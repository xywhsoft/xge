#include "uid_document.h"
#include <string.h>

void UIDesignDocumentInit(uid_document_t* pDocument)
{
	if ( pDocument == NULL ) {
		return;
	}
	memset(pDocument, 0, sizeof(*pDocument));
	strcpy(pDocument->sName, "Untitled");
}

void UIDesignDocumentUnit(uid_document_t* pDocument)
{
	(void)pDocument;
}

void UIDesignDocumentMarkDirty(uid_document_t* pDocument)
{
	if ( pDocument == NULL ) {
		return;
	}
	pDocument->iRevision++;
	pDocument->bDirty = (pDocument->iRevision != pDocument->iCleanRevision);
}
