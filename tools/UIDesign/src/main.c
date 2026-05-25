#include "uid_app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int UIDesignArgInt(const char* sText, int iDefault)
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
	static uid_app_t tApp;
	int i;
	int iExitCode;

	memset(&tDesc, 0, sizeof(tDesc));
	if ( UIDesignAppInit(&tApp) != XGE_OK ) {
		return 1;
	}
	tApp.iFrameLimit = UIDesignArgInt(getenv("XGE_UIDESIGN_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = UIDesignArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.sTitle = "XUI Designer";
	tDesc.iWidth = 1400;
	tDesc.iHeight = 900;
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	if ( tApp.iFrameLimit <= 0 ) {
		tDesc.iFlags |= XGE_INIT_ON_DEMAND;
	}
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 30;
	tDesc.pUser = &tApp;
	tApp.tScene.onEnter = UIDesignAppEnter;
	tApp.tScene.onLeave = UIDesignAppLeave;
	tApp.tScene.onEvent = UIDesignAppEvent;
	tApp.tScene.onUpdate = UIDesignAppUpdate;
	tApp.tScene.onDraw = UIDesignAppDraw;
	tApp.tScene.pUser = &tApp;
	iExitCode = 0;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		printf("uidesign init failed\n");
		UIDesignAppUnit(&tApp);
		return 1;
	}
	if ( xgeSceneSet(&tApp.tScene) != XGE_OK ) {
		xgeUnit();
		UIDesignAppUnit(&tApp);
		return 2;
	}
	iExitCode = xgeRun(NULL, NULL);
	xgeUnit();
	UIDesignAppUnit(&tApp);
	return iExitCode;
}
