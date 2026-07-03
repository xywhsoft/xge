#ifndef UID_DOCUMENT_H
#define UID_DOCUMENT_H

#include "xge.h"

#define UID_DOCUMENT_NAME_CAPACITY 128
#define UID_DOCUMENT_PATH_CAPACITY 512
#define UID_DOCUMENT_NODE_CAPACITY 128
#define UID_NODE_TYPE_CAPACITY 32
#define UID_NODE_ID_CAPACITY 64
#define UID_NODE_TEXT_CAPACITY 128

typedef struct uid_node_t {
	int iId;
	char sType[UID_NODE_TYPE_CAPACITY];
	char sName[UID_NODE_ID_CAPACITY];
	char sText[UID_NODE_TEXT_CAPACITY];
	xge_rect_t tRect;
} uid_node_t;

typedef struct uid_document_t {
	char sName[UID_DOCUMENT_NAME_CAPACITY];
	char sPath[UID_DOCUMENT_PATH_CAPACITY];
	uint32_t iRevision;
	uint32_t iCleanRevision;
	int bDirty;
	uid_node_t arrNodes[UID_DOCUMENT_NODE_CAPACITY];
	int iNodeCount;
	int iNextNodeId;
	int iSelectedNode;
} uid_document_t;

void UIDesignDocumentInit(uid_document_t* pDocument);
void UIDesignDocumentUnit(uid_document_t* pDocument);
void UIDesignDocumentMarkDirty(uid_document_t* pDocument);
xge_rect_t UIDesignDocumentDefaultRect(const char* sType);
uid_node_t* UIDesignDocumentAddNode(uid_document_t* pDocument, const char* sType);
uid_node_t* UIDesignDocumentGetSelected(uid_document_t* pDocument);

#endif
