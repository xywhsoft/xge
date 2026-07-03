#include "ske_app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif

static int SkeletonEditArgInt(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iValue : iDefault;
}

static void SkeletonEditSetWorkingDirectory(void)
{
#ifdef _WIN32
	char sPath[MAX_PATH];
	char* pSlash;

	if ( GetModuleFileNameA(NULL, sPath, sizeof(sPath)) == 0 ) {
		return;
	}
	sPath[sizeof(sPath) - 1] = 0;
	pSlash = strrchr(sPath, '\\');
	if ( pSlash == NULL ) {
		pSlash = strrchr(sPath, '/');
	}
	if ( pSlash == NULL ) {
		return;
	}
	*pSlash = 0;
	SetCurrentDirectoryA(sPath);
#endif
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	static ske_app_t tApp;
	int i;
	int iExitCode;

	SkeletonEditSetWorkingDirectory();
	memset(&tDesc, 0, sizeof(tDesc));
	if ( SkeletonEditAppInit(&tApp) != XGE_OK ) {
		return 1;
	}
	tApp.iFrameLimit = SkeletonEditArgInt(getenv("XGE_SKELETONEDIT_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = SkeletonEditArgInt(argv[++i], tApp.iFrameLimit);
		} else if ( strcmp(argv[i], "--smoke") == 0 ) {
			tApp.bSmoke = 1;
		}
	}
	if ( (tApp.iFrameLimit <= 0) && tApp.bSmoke ) {
		tApp.iFrameLimit = 2;
	}
	tDesc.sTitle = "SkeletonEdit";
	tDesc.iWidth = 1400;
	tDesc.iHeight = 900;
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	if ( tApp.iFrameLimit <= 0 ) {
		tDesc.iFlags |= XGE_INIT_ON_DEMAND;
	}
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 30;
	tDesc.pUser = &tApp;
	tApp.tScene.onEnter = SkeletonEditAppEnter;
	tApp.tScene.onLeave = SkeletonEditAppLeave;
	tApp.tScene.onEvent = SkeletonEditAppEvent;
	tApp.tScene.onUpdate = SkeletonEditAppUpdate;
	tApp.tScene.onDraw = SkeletonEditAppDraw;
	tApp.tScene.pUser = &tApp;
	iExitCode = 0;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		printf("skeleton_edit init failed\n");
		SkeletonEditAppUnit(&tApp);
		return 1;
	}
	if ( xgeSceneSet(&tApp.tScene) != XGE_OK ) {
		xgeUnit();
		SkeletonEditAppUnit(&tApp);
		return 2;
	}
	iExitCode = xgeRun(NULL, NULL);
	xgeUnit();
	SkeletonEditAppUnit(&tApp);
	return iExitCode;
}
