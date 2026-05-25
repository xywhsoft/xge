#include "uid_project.h"
#include <string.h>

void UIDesignProjectInit(uid_project_t* pProject)
{
	if ( pProject == NULL ) {
		return;
	}
	memset(pProject, 0, sizeof(*pProject));
	strcpy(pProject->sName, "Untitled Project");
	pProject->iPreviewWidth = 1366;
	pProject->iPreviewHeight = 768;
	pProject->fPreviewDipScale = 1.0f;
}

void UIDesignProjectUnit(uid_project_t* pProject)
{
	(void)pProject;
}
