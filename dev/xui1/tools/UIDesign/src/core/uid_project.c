#include "uid_project.h"
#include "uid_document.h"
#include <direct.h>
#include <stdio.h>
#include <string.h>

static void UIDesignProjectCopyText(char* sDst, int iDstSize, const char* sSrc)
{
	size_t iLen;

	if ( (sDst == NULL) || (iDstSize <= 0) ) {
		return;
	}
	if ( sSrc == NULL ) {
		sSrc = "";
	}
	iLen = strlen(sSrc);
	if ( iLen >= (size_t)iDstSize ) {
		iLen = (size_t)iDstSize - 1u;
	}
	memmove(sDst, sSrc, iLen);
	sDst[iLen] = 0;
}

static void UIDesignProjectMakeDir(const char* sPath)
{
	if ( (sPath != NULL) && (sPath[0] != 0) ) {
		_mkdir(sPath);
	}
}

static int UIDesignProjectWriteTextIfMissing(const char* sPath, const char* sText)
{
	FILE* pFile;

	pFile = fopen(sPath, "rb");
	if ( pFile != NULL ) {
		fclose(pFile);
		return XGE_OK;
	}
	pFile = fopen(sPath, "wb");
	if ( pFile == NULL ) {
		return XGE_ERROR;
	}
	if ( sText != NULL ) {
		fputs(sText, pFile);
	}
	fclose(pFile);
	return XGE_OK;
}

void UIDesignProjectInit(uid_project_t* pProject)
{
	if ( pProject == NULL ) {
		return;
	}
	memset(pProject, 0, sizeof(*pProject));
	strcpy(pProject->sName, "Untitled Project");
	strcpy(pProject->sCurrentPage, "pages/main.xson");
	pProject->iPreviewWidth = 1366;
	pProject->iPreviewHeight = 768;
	pProject->fPreviewDipScale = 1.0f;
}

void UIDesignProjectUnit(uid_project_t* pProject)
{
	(void)pProject;
}

int UIDesignProjectEnsureDefault(uid_project_t* pProject)
{
	char sPath[UID_PROJECT_PATH_CAPACITY];

	if ( pProject == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	UIDesignProjectCopyText(pProject->sName, sizeof(pProject->sName), "DemoProject");
	UIDesignProjectCopyText(pProject->sRootPath, sizeof(pProject->sRootPath), "workspace/DemoProject");
	UIDesignProjectCopyText(pProject->sCurrentPage, sizeof(pProject->sCurrentPage), "pages/main.xson");
	UIDesignProjectMakeDir("workspace");
	UIDesignProjectMakeDir(pProject->sRootPath);
	snprintf(sPath, sizeof(sPath), "%s/pages", pProject->sRootPath);
	UIDesignProjectMakeDir(sPath);
	snprintf(sPath, sizeof(sPath), "%s/styles", pProject->sRootPath);
	UIDesignProjectMakeDir(sPath);
	snprintf(sPath, sizeof(sPath), "%s/res", pProject->sRootPath);
	UIDesignProjectMakeDir(sPath);
	snprintf(sPath, sizeof(sPath), "%s/assets", pProject->sRootPath);
	UIDesignProjectMakeDir(sPath);
	snprintf(sPath, sizeof(sPath), "%s/project.xson", pProject->sRootPath);
	if ( UIDesignProjectWriteTextIfMissing(sPath,
		"{\n"
		"  name: \"DemoProject\",\n"
		"  pages: [ \"pages/main.xson\" ],\n"
		"  styles: [ \"styles/theme.xson\" ],\n"
		"  assets: [ \"res\", \"assets\" ]\n"
		"}\n") != XGE_OK ) {
		return XGE_ERROR;
	}
	snprintf(sPath, sizeof(sPath), "%s/styles/theme.xson", pProject->sRootPath);
	if ( UIDesignProjectWriteTextIfMissing(sPath, "{\n  styles: {}\n}\n") != XGE_OK ) {
		return XGE_ERROR;
	}
	snprintf(sPath, sizeof(sPath), "%s/pages/main.xson", pProject->sRootPath);
	if ( UIDesignProjectWriteTextIfMissing(sPath, "{\n  page: \"main\",\n  controls: []\n}\n") != XGE_OK ) {
		return XGE_ERROR;
	}
	pProject->bReady = 1;
	return XGE_OK;
}

int UIDesignProjectSaveDocument(uid_project_t* pProject, const uid_document_t* pDocument)
{
	FILE* pFile;
	char sPath[UID_PROJECT_PATH_CAPACITY];
	const uid_node_t* pNode;
	int i;

	if ( (pProject == NULL) || (pDocument == NULL) || !pProject->bReady ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	snprintf(sPath, sizeof(sPath), "%s/%s", pProject->sRootPath, pProject->sCurrentPage);
	pFile = fopen(sPath, "wb");
	if ( pFile == NULL ) {
		return XGE_ERROR;
	}
	fprintf(pFile, "{\n");
	fprintf(pFile, "  page: \"%s\",\n", pDocument->sName[0] ? pDocument->sName : "main");
	fprintf(pFile, "  controls: [\n");
	for ( i = 0; i < pDocument->iNodeCount; i++ ) {
		pNode = &pDocument->arrNodes[i];
		fprintf(pFile, "    { id: %d, type: \"%s\", name: \"%s\", text: \"%s\", rect: { x: %.0f, y: %.0f, w: %.0f, h: %.0f } }%s\n",
			pNode->iId, pNode->sType, pNode->sName, pNode->sText,
			pNode->tRect.fX, pNode->tRect.fY, pNode->tRect.fW, pNode->tRect.fH,
			(i + 1 < pDocument->iNodeCount) ? "," : "");
	}
	fprintf(pFile, "  ]\n");
	fprintf(pFile, "}\n");
	fclose(pFile);
	return XGE_OK;
}
