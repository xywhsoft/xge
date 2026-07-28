/*
 * player_app.c - AnimPlayer playback control and application logic
 *
 * Manages the player state, keyboard/mouse input for playback
 * control, speed adjustment, and frame navigation.
 */

#include "player_app.h"
#include "xanim.h"
#include "xge.h"
#include <stdio.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Player application state                                           */
/* ------------------------------------------------------------------ */

static player_app_t s_tApp;

/* ------------------------------------------------------------------ */
/* Init / Shutdown                                                    */
/* ------------------------------------------------------------------ */

int playerAppInit(player_app_t* pApp, const char* sFile)
{
	memset(pApp, 0, sizeof(*pApp));

	pApp->fSpeed = 1.0f;
	pApp->bLoop = 1;
	pApp->bShowInfo = 1;
	pApp->bShowControls = 1;

	/* Create player */
	if ( xanimPlayerCreate(&pApp->pPlayer) != 0 ) {
		printf("AnimPlayer: failed to create player\n");
		return -1;
	}

	/* Load file */
	if ( sFile && sFile[0] ) {
		if ( xanimPlayerLoad(pApp->pPlayer, sFile) != 0 ) {
			printf("AnimPlayer: failed to load '%s'\n", sFile);
			xanimPlayerDestroy(pApp->pPlayer);
			pApp->pPlayer = NULL;
			return -2;
		}
		strncpy(pApp->sFilePath, sFile, sizeof(pApp->sFilePath) - 1);
		xanimPlayerSetLoop(pApp->pPlayer, pApp->bLoop);
		xanimPlayerPlay(pApp->pPlayer);
		pApp->bPlaying = 1;
	}

	s_tApp = *pApp;
	return 0;
}

void playerAppShutdown(player_app_t* pApp)
{
	if ( pApp->pPlayer ) {
		xanimPlayerStop(pApp->pPlayer);
		xanimPlayerDestroy(pApp->pPlayer);
		pApp->pPlayer = NULL;
	}
}

/* ------------------------------------------------------------------ */
/* Frame update                                                       */
/* ------------------------------------------------------------------ */

int playerAppUpdate(player_app_t* pApp, float fDelta)
{
	if ( pApp->pPlayer == NULL )
		return 0;

	/* Update playback with speed multiplier */
	xanimPlayerSetSpeed(pApp->pPlayer, pApp->fSpeed);
	xanimPlayerUpdate(pApp->pPlayer, fDelta);

	/* Sync state */
	pApp->iCurrentFrame = (int)xanimPlayerGetFrame(pApp->pPlayer);
	pApp->iFrameCount = (int)xanimPlayerGetFrameCount(pApp->pPlayer);
	pApp->bPlaying = xanimPlayerIsPlaying(pApp->pPlayer);

	return 0;
}

/* ------------------------------------------------------------------ */
/* Playback controls                                                  */
/* ------------------------------------------------------------------ */

void playerAppTogglePlay(player_app_t* pApp)
{
	if ( pApp->pPlayer == NULL ) return;

	if ( pApp->bPlaying ) {
		xanimPlayerPause(pApp->pPlayer);
		pApp->bPlaying = 0;
	} else {
		xanimPlayerPlay(pApp->pPlayer);
		pApp->bPlaying = 1;
	}
}

void playerAppStop(player_app_t* pApp)
{
	if ( pApp->pPlayer == NULL ) return;
	xanimPlayerStop(pApp->pPlayer);
	pApp->bPlaying = 0;
	pApp->iCurrentFrame = 0;
}

void playerAppStepForward(player_app_t* pApp)
{
	if ( pApp->pPlayer == NULL ) return;
	xanimPlayerPause(pApp->pPlayer);
	pApp->bPlaying = 0;
	pApp->iCurrentFrame++;
	if ( pApp->iCurrentFrame >= pApp->iFrameCount )
		pApp->iCurrentFrame = 0;
	xanimPlayerGotoFrame(pApp->pPlayer, (uint32_t)pApp->iCurrentFrame);
}

void playerAppStepBackward(player_app_t* pApp)
{
	if ( pApp->pPlayer == NULL ) return;
	xanimPlayerPause(pApp->pPlayer);
	pApp->bPlaying = 0;
	pApp->iCurrentFrame--;
	if ( pApp->iCurrentFrame < 0 )
		pApp->iCurrentFrame = pApp->iFrameCount - 1;
	xanimPlayerGotoFrame(pApp->pPlayer, (uint32_t)pApp->iCurrentFrame);
}

void playerAppGotoFirst(player_app_t* pApp)
{
	if ( pApp->pPlayer == NULL ) return;
	xanimPlayerGotoFrame(pApp->pPlayer, 0);
	pApp->iCurrentFrame = 0;
}

void playerAppGotoLast(player_app_t* pApp)
{
	if ( pApp->pPlayer == NULL ) return;
	if ( pApp->iFrameCount > 0 ) {
		xanimPlayerGotoFrame(pApp->pPlayer, (uint32_t)(pApp->iFrameCount - 1));
		pApp->iCurrentFrame = pApp->iFrameCount - 1;
	}
}

void playerAppToggleLoop(player_app_t* pApp)
{
	pApp->bLoop = !pApp->bLoop;
	if ( pApp->pPlayer )
		xanimPlayerSetLoop(pApp->pPlayer, pApp->bLoop);
}

void playerAppSetSpeed(player_app_t* pApp, float fSpeed)
{
	if ( fSpeed < 0.25f ) fSpeed = 0.25f;
	if ( fSpeed > 4.0f ) fSpeed = 4.0f;
	pApp->fSpeed = fSpeed;
}

void playerAppCycleSpeed(player_app_t* pApp)
{
	/* Cycle through common speeds */
	float speeds[] = { 0.25f, 0.5f, 1.0f, 1.5f, 2.0f, 4.0f };
	int count = 6;
	int i;

	for ( i = 0; i < count; i++ ) {
		if ( pApp->fSpeed < speeds[i] - 0.01f ) {
			pApp->fSpeed = speeds[i];
			return;
		}
	}
	pApp->fSpeed = speeds[0]; /* wrap around */
}

/* ------------------------------------------------------------------ */
/* Keyboard handling                                                  */
/* ------------------------------------------------------------------ */

int playerAppHandleKey(player_app_t* pApp, int iKey)
{
	switch ( iKey ) {
	case ' ':       /* Space = play/pause */
		playerAppTogglePlay(pApp);
		return 1;

	case 262:       /* Right = step forward */
		playerAppStepForward(pApp);
		return 1;

	case 263:       /* Left = step backward */
		playerAppStepBackward(pApp);
		return 1;

	case 268:       /* Home = first frame */
		playerAppGotoFirst(pApp);
		return 1;

	case 269:       /* End = last frame */
		playerAppGotoLast(pApp);
		return 1;

	case 'L':       /* L = toggle loop */
	case 'l':
		playerAppToggleLoop(pApp);
		return 1;

	case 'S':       /* S = cycle speed */
	case 's':
		playerAppCycleSpeed(pApp);
		return 1;

	case 'I':       /* I = toggle info */
	case 'i':
		pApp->bShowInfo = !pApp->bShowInfo;
		return 1;

	case 'H':       /* H = toggle controls */
	case 'h':
		pApp->bShowControls = !pApp->bShowControls;
		return 1;

	case 256:       /* Escape = quit */
		return -1;

	default:
		break;
	}

	return 0;
}

/* ------------------------------------------------------------------ */
/* File loading                                                       */
/* ------------------------------------------------------------------ */

int playerAppLoadFile(player_app_t* pApp, const char* sPath)
{
	if ( pApp->pPlayer == NULL ) return -1;

	xanimPlayerStop(pApp->pPlayer);

	if ( xanimPlayerLoad(pApp->pPlayer, sPath) != 0 ) {
		printf("AnimPlayer: failed to load '%s'\n", sPath);
		return -2;
	}

	strncpy(pApp->sFilePath, sPath, sizeof(pApp->sFilePath) - 1);
	pApp->iFrameCount = (int)xanimPlayerGetFrameCount(pApp->pPlayer);
	pApp->iCurrentFrame = 0;
	pApp->bPlaying = 1;

	xanimPlayerSetLoop(pApp->pPlayer, pApp->bLoop);
	xanimPlayerPlay(pApp->pPlayer);

	printf("AnimPlayer: loaded '%s' (%d frames @ %.1f fps)\n",
	       sPath, pApp->iFrameCount, xanimPlayerGetFrameRate(pApp->pPlayer));

	return 0;
}
