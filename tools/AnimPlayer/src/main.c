/*
 * AnimPlayer - Standalone .xanim file player
 *
 * Minimal XGE application that loads and plays .xanim animations.
 * External plugin code depending on xge.dll.
 *
 * Usage: AnimPlayer.exe <file.xanim> [--loop] [--speed=N]
 */

#include "xge.h"
#include "xanim.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Global state                                                       */
/* ------------------------------------------------------------------ */

static xanim_player g_pPlayer = NULL;
static int g_bLoop = 1;
static float g_fSpeed = 1.0f;

/* ------------------------------------------------------------------ */
/* Render callback                                                    */
/* ------------------------------------------------------------------ */

static void __onRender(void)
{
	float fW, fH;
	float fWinW = (float)xgeGetWidth();
	float fWinH = (float)xgeGetHeight();
	float fScaleX, fScaleY, fScale;
	float fOffX, fOffY;

	xgeClear(XGE_COLOR_RGBA(18, 18, 18, 255));

	if ( g_pPlayer != NULL ) {
		xanimPlayerGetSize(g_pPlayer, &fW, &fH);

		/* Fit animation to window */
		fScaleX = fWinW / fW;
		fScaleY = fWinH / fH;
		fScale = (fScaleX < fScaleY) ? fScaleX : fScaleY;
		if ( fScale > 1.0f ) fScale = 1.0f;  /* don't upscale */
		fOffX = (fWinW - fW * fScale) * 0.5f;
		fOffY = (fWinH - fH * fScale) * 0.5f;

		/* Draw stage background (white) */
		{
			xge_shape_ex pBg;
			if ( xgeShapeExCreate(&pBg) == XGE_OK ) {
				xgeShapeExAppendRect(pBg, fOffX, fOffY, fW * fScale, fH * fScale, 0, 0, 1);
				xgeShapeExFillColor(pBg, XGE_COLOR_RGBA(255, 255, 255, 255));
				xgeShapeExDraw(pBg, 1.0f);
				xgeShapeExDestroy(pBg);
			}
		}

		/* Render animation */
		/* Note: for proper scaling we'd use RenderEx with matrix; simplified here */
		xanimPlayerRender(g_pPlayer, fOffX, fOffY);
	}
}

/* ------------------------------------------------------------------ */
/* Frame callback (called by xgeRun)                                  */
/* ------------------------------------------------------------------ */

static int __onFrame(void* pUser)
{
	float fDelta = xgeGetDelta();
	(void)pUser;

	/* Handle input */
	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
		return XGE_OK;
	}
	if ( xgeKeyPressed(XGE_KEY_SPACE) ) {
		if ( xanimPlayerIsPlaying(g_pPlayer) ) xanimPlayerPause(g_pPlayer);
		else xanimPlayerPlay(g_pPlayer);
	}
	if ( xgeKeyPressed(XGE_KEY_RIGHT) ) {
		xanimPlayerGotoFrame(g_pPlayer, xanimPlayerGetFrame(g_pPlayer) + 1);
	}
	if ( xgeKeyPressed(XGE_KEY_LEFT) ) {
		uint32_t f = xanimPlayerGetFrame(g_pPlayer);
		xanimPlayerGotoFrame(g_pPlayer, (f > 0) ? f - 1 : 0);
	}
	if ( xgeKeyPressed(XGE_KEY_HOME) ) {
		xanimPlayerGotoFrame(g_pPlayer, 0);
	}
	if ( xgeKeyPressed('L') ) {
		g_bLoop = !g_bLoop;
		xanimPlayerSetLoop(g_pPlayer, g_bLoop);
	}

	/* Update playback */
	if ( g_pPlayer != NULL ) {
		xanimPlayerUpdate(g_pPlayer, fDelta);
	}

	/* Render */
	xgeBegin();
	__onRender();
	xgeEnd();

	xgeRenderRequest();
	return XGE_OK;
}

/* ------------------------------------------------------------------ */
/* Entry point                                                        */
/* ------------------------------------------------------------------ */

int main(int argc, char** argv)
{
	const char* sFile = NULL;
	int i;

	/* Parse arguments */
	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--loop") == 0 ) {
			g_bLoop = 1;
		} else if ( strncmp(argv[i], "--speed=", 8) == 0 ) {
			g_fSpeed = (float)atof(argv[i] + 8);
			if ( g_fSpeed <= 0.0f ) g_fSpeed = 1.0f;
		} else if ( argv[i][0] != '-' ) {
			sFile = argv[i];
		}
	}

	if ( sFile == NULL ) {
		printf("AnimPlayer - XANIM Animation Player\n");
		printf("Usage: AnimPlayer.exe <file.xanim> [--loop] [--speed=N]\n");
		printf("\nControls:\n");
		printf("  Space  - Play/Pause\n");
		printf("  Left/Right - Step frame\n");
		printf("  Home   - Go to first frame\n");
		printf("  L      - Toggle loop\n");
		printf("  Escape - Quit\n");
		return 1;
	}

	/* Create player and load file */
	if ( xanimPlayerCreate(&g_pPlayer) != 0 ) {
		fprintf(stderr, "Error: Failed to create player\n");
		return 1;
	}
	if ( xanimPlayerLoad(g_pPlayer, sFile) != 0 ) {
		fprintf(stderr, "Error: Failed to load '%s'\n", sFile);
		xanimPlayerDestroy(g_pPlayer);
		return 1;
	}

	xanimPlayerSetLoop(g_pPlayer, g_bLoop);
	xanimPlayerSetSpeed(g_pPlayer, g_fSpeed);

	/* Print info */
	{
		float fW, fH;
		xanimPlayerGetSize(g_pPlayer, &fW, &fH);
		printf("AnimPlayer: %s\n", sFile);
		printf("  Size: %.0f x %.0f\n", fW, fH);
		printf("  Frames: %u @ %.1f fps\n", xanimPlayerGetFrameCount(g_pPlayer), xanimPlayerGetFrameRate(g_pPlayer));
		printf("  Symbols: %d\n", xanimPlayerGetSymbolCount(g_pPlayer));
	}

	/* Initialize XGE */
	{
		xge_desc_t tDesc;
		memset(&tDesc, 0, sizeof(tDesc));
		tDesc.sTitle = "AnimPlayer";
		tDesc.iWidth = 800;
		tDesc.iHeight = 600;
		tDesc.iFlags = XGE_INIT_RESIZABLE | XGE_INIT_VSYNC;
		tDesc.iRunMode = XGE_RUN_GAME_LOOP;
		tDesc.iTargetFPS = 60;

		if ( xgeInit(&tDesc) != XGE_OK ) {
			fprintf(stderr, "Error: XGE init failed\n");
			xanimPlayerDestroy(g_pPlayer);
			return 1;
		}
	}

	/* Start playing */
	xanimPlayerPlay(g_pPlayer);

	/* Run main loop */
	xgeRun(__onFrame, NULL);

	/* Cleanup */
	xanimPlayerDestroy(g_pPlayer);
	g_pPlayer = NULL;
	xgeUnit();
	return 0;
}
