#include "mapedit_app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int ArgInt(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iValue : iDefault;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	static mapedit_app_t tApp;
	int i;
	int iExitCode;

	memset(&tDesc, 0, sizeof(tDesc));
	if ( MapEditAppInit(&tApp) != XGE_OK ) {
		return 1;
	}
	tApp.iFrameLimit = ArgInt(getenv("XGE_MAPEDIT_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.sTitle = "MapEdit";
	tDesc.iWidth = 1400;
	tDesc.iHeight = 900;
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	tDesc.pUser = &tApp;
	tApp.tScene.onEnter = MapEditAppEnter;
	tApp.tScene.onLeave = MapEditAppLeave;
	tApp.tScene.onEvent = MapEditAppEvent;
	tApp.tScene.onUpdate = MapEditAppUpdate;
	tApp.tScene.onDraw = MapEditAppDraw;
	tApp.tScene.pUser = &tApp;
	iExitCode = 0;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		printf("mapedit init failed\n");
		MapEditAppUnit(&tApp);
		return 1;
	}
	if ( xgeSceneSet(&tApp.tScene) != XGE_OK ) {
		xgeUnit();
		MapEditAppUnit(&tApp);
		return 2;
	}
	iExitCode = xgeRun(NULL, NULL);
	xgeUnit();
	MapEditAppUnit(&tApp);
	return iExitCode;
}
