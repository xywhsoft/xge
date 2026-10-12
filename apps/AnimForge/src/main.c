/*
 * AnimForge - Flash-like animation editor built on XGE/XUI
 *
 * External tool: links against xge.dll, does NOT modify engine source.
 * Entry point + XGE initialization + frame loop.
 */

#include "core/af_app.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Command-line parsing                                               */
/* ------------------------------------------------------------------ */

static void __afUsage(void)
{
	printf("usage: AnimForge [--frames N] [--seconds N] [file.xanim]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __afParseArgs(af_app_t* pApp, int argc, char** argv)
{
	int i;
	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 ) {
			if ( i + 1 >= argc ) return -1;
			pApp->iMaxFrames = atoi(argv[++i]);
			if ( pApp->iMaxFrames <= 0 ) return -1;
		} else if ( strncmp(argv[i], "--frames=", 9) == 0 ) {
			pApp->iMaxFrames = atoi(argv[i] + 9);
			if ( pApp->iMaxFrames <= 0 ) return -1;
		} else if ( strcmp(argv[i], "--seconds") == 0 ) {
			if ( i + 1 >= argc ) return -1;
			pApp->fMaxSeconds = atof(argv[++i]);
			if ( pApp->fMaxSeconds <= 0.0 ) return -1;
		} else if ( strncmp(argv[i], "--seconds=", 10) == 0 ) {
			pApp->fMaxSeconds = atof(argv[i] + 10);
			if ( pApp->fMaxSeconds <= 0.0 ) return -1;
		} else if ( strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 ) {
			__afUsage();
			return 1;
		} else if ( argv[i][0] != '-' ) {
			/* Positional arg: file to open (deferred until after init) */
		} else {
			return -1;
		}
	}
	return 0;
}

/* ------------------------------------------------------------------ */
/* Frame callback (called by xgeRun each frame)                       */
/* ------------------------------------------------------------------ */

static int __afFrameCallback(void* pUser)
{
	af_app_t* pApp = (af_app_t*)pUser;
	int ret;

	if ( pApp == NULL ) return XGE_ERROR_INVALID_ARGUMENT;

	/* Update logic + input + layout */
	ret = afAppFrame(pApp);
	if ( ret != XUI_OK ) {
		printf("AnimForge: frame error %d\n", ret);
		xgeQuit();
		return ret;
	}

	/* Render */
	afAppRender(pApp);

	/* Duration limit check */
	pApp->iFrame++;
	if ( (pApp->iMaxFrames > 0 && pApp->iFrame >= pApp->iMaxFrames) ||
	     (pApp->fMaxSeconds > 0.0 && xgeTimer() >= pApp->fMaxSeconds) ) {
		printf("AnimForge: completed %d frames\n", pApp->iFrame);
		xgeQuit();
	}

	return XGE_OK;
}

/* ------------------------------------------------------------------ */
/* main                                                               */
/* ------------------------------------------------------------------ */

/* ------------------------------------------------------------------ */
/* Pause on error (so double-click users can read the message)        */
/* ------------------------------------------------------------------ */

static void __afPauseOnError(void)
{
	fflush(stdout);
	fflush(stderr);
	fprintf(stderr, "\nPress Enter to exit...\n");
	fflush(stderr);
	getchar();
}

int main(int argc, char** argv)
{
	af_app_t* pApp;
	xge_desc_t desc;
	int ret;

	/* Heap-allocate app state (struct is too large for stack) */
	pApp = (af_app_t*)calloc(1, sizeof(af_app_t));
	if ( pApp == NULL ) {
		printf("AnimForge: out of memory\n");
		__afPauseOnError();
		return 1;
	}

	/* Init app state */
	ret = afAppInit(pApp);
	if ( ret != 0 ) {
		printf("AnimForge: afAppInit failed: %d\n", ret);
		__afPauseOnError();
		free(pApp);
		return 1;
	}

	/* Parse command line */
	ret = __afParseArgs(pApp, argc, argv);
	if ( ret == 1 ) { free(pApp); return 0; }  /* --help */
	if ( ret != 0 ) {
		__afUsage();
		free(pApp);
		return 1;
	}

	/* Initialize XGE window */
	memset(&desc, 0, sizeof(desc));
	desc.iWidth = AF_APP_WIDTH;
	desc.iHeight = AF_APP_HEIGHT;
	desc.sTitle = AF_APP_TITLE;
	desc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	desc.iRunMode = XGE_RUN_GAME_LOOP;
	desc.iTargetFPS = 60;

	ret = xgeInit(&desc);
	if ( ret != XGE_OK ) {
		printf("AnimForge: xgeInit failed: %d\n", ret);
		printf("  Possible causes: no OpenGL 3.3 GPU, or display not available.\n");
		__afPauseOnError();
		free(pApp);
		return 1;
	}

	/* Create XUI assets and workspace */
	ret = afAppCreateUi(pApp);
	if ( ret != XUI_OK ) {
		printf("AnimForge: UI creation failed: %d\n", ret);
		afAppShutdown(pApp);
		xgeUnit();
		__afPauseOnError();
		free(pApp);
		return 1;
	}

	/* Create a default document (640x480 @ 24fps, 100 frames) */
	afAppNewDocument(pApp, 640.0f, 480.0f, 24.0f, 100);

	printf("AnimForge: ready. Stage 640x480 @ 24fps\n");

	/* Run main loop */
	ret = xgeRun(__afFrameCallback, pApp);

	/* Cleanup */
	afAppShutdown(pApp);
	xgeUnit();
	free(pApp);

	return (ret == XGE_OK) ? 0 : 1;
}
