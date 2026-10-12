/*
 * player_shell.c - AnimPlayer window / menu / drag-drop
 *
 * Handles the player window chrome: title bar info overlay,
 * control bar rendering, file drag-drop, and playlist.
 */

#include "player_app.h"
#include "xge.h"
#include <stdio.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Playlist                                                           */
/* ------------------------------------------------------------------ */

#define PLAYER_MAX_PLAYLIST  64

typedef struct player_playlist_t {
	char arrFiles[PLAYER_MAX_PLAYLIST][512];
	int  iCount;
	int  iCurrent;
} player_playlist_t;

static player_playlist_t s_tPlaylist;

void playerShellInitPlaylist(void)
{
	memset(&s_tPlaylist, 0, sizeof(s_tPlaylist));
	s_tPlaylist.iCurrent = -1;
}

int playerShellAddToPlaylist(const char* sPath)
{
	if ( s_tPlaylist.iCount >= PLAYER_MAX_PLAYLIST )
		return -1;
	strncpy(s_tPlaylist.arrFiles[s_tPlaylist.iCount], sPath, 511);
	s_tPlaylist.arrFiles[s_tPlaylist.iCount][511] = '\0';
	s_tPlaylist.iCount++;
	if ( s_tPlaylist.iCurrent < 0 )
		s_tPlaylist.iCurrent = 0;
	return 0;
}

const char* playerShellGetNextFile(void)
{
	if ( s_tPlaylist.iCount == 0 )
		return NULL;
	s_tPlaylist.iCurrent++;
	if ( s_tPlaylist.iCurrent >= s_tPlaylist.iCount )
		s_tPlaylist.iCurrent = 0;
	return s_tPlaylist.arrFiles[s_tPlaylist.iCurrent];
}

const char* playerShellGetPrevFile(void)
{
	if ( s_tPlaylist.iCount == 0 )
		return NULL;
	s_tPlaylist.iCurrent--;
	if ( s_tPlaylist.iCurrent < 0 )
		s_tPlaylist.iCurrent = s_tPlaylist.iCount - 1;
	return s_tPlaylist.arrFiles[s_tPlaylist.iCurrent];
}

int playerShellGetPlaylistCount(void)
{
	return s_tPlaylist.iCount;
}

/* ------------------------------------------------------------------ */
/* Info overlay rendering (using ShapeEx)                             */
/* ------------------------------------------------------------------ */

void playerShellRenderInfo(player_app_t* pApp, float fWinW, float fWinH)
{
	xge_shape_ex pShape;
	float stageW, stageH;

	if ( !pApp->bShowInfo )
		return;

	if ( pApp->pPlayer == NULL )
		return;

	xanimPlayerGetSize(pApp->pPlayer, &stageW, &stageH);

	/* Draw info panel background (top-left) */
	if ( xgeShapeExCreate(&pShape) != XGE_OK )
		return;

	xgeShapeExMoveTo(pShape, 8, 8);
	xgeShapeExLineTo(pShape, 220, 8);
	xgeShapeExLineTo(pShape, 220, 80);
	xgeShapeExLineTo(pShape, 8, 80);
	xgeShapeExClose(pShape);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(0, 0, 0, 160));
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* Info text would be rendered via XGE font system.
	 * Content: filename, frame X/Y, fps, speed, loop state */
	(void)fWinW; (void)fWinH;
}

/* ------------------------------------------------------------------ */
/* Control bar rendering                                              */
/* ------------------------------------------------------------------ */

void playerShellRenderControls(player_app_t* pApp, float fWinW, float fWinH)
{
	xge_shape_ex pShape;
	float barY = fWinH - 40.0f;
	float barH = 32.0f;

	if ( !pApp->bShowControls )
		return;

	/* Control bar background */
	if ( xgeShapeExCreate(&pShape) != XGE_OK )
		return;

	xgeShapeExMoveTo(pShape, 0, barY);
	xgeShapeExLineTo(pShape, fWinW, barY);
	xgeShapeExLineTo(pShape, fWinW, barY + barH);
	xgeShapeExLineTo(pShape, 0, barY + barH);
	xgeShapeExClose(pShape);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(30, 30, 30, 200));
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* Progress bar */
	if ( pApp->iFrameCount > 0 ) {
		float progress = (float)pApp->iCurrentFrame / (float)pApp->iFrameCount;
		float barX = 60.0f;
		float barW = fWinW - 120.0f;

		/* Track */
		if ( xgeShapeExCreate(&pShape) != XGE_OK ) return;
		xgeShapeExMoveTo(pShape, barX, barY + 14);
		xgeShapeExLineTo(pShape, barX + barW, barY + 14);
		xgeShapeExLineTo(pShape, barX + barW, barY + 18);
		xgeShapeExLineTo(pShape, barX, barY + 18);
		xgeShapeExClose(pShape);
		xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(80, 80, 80, 255));
		xgeShapeExDraw(pShape, 0.5f);
		xgeShapeExDestroy(pShape);

		/* Filled portion */
		if ( xgeShapeExCreate(&pShape) != XGE_OK ) return;
		xgeShapeExMoveTo(pShape, barX, barY + 14);
		xgeShapeExLineTo(pShape, barX + barW * progress, barY + 14);
		xgeShapeExLineTo(pShape, barX + barW * progress, barY + 18);
		xgeShapeExLineTo(pShape, barX, barY + 18);
		xgeShapeExClose(pShape);
		xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(0, 122, 204, 255));
		xgeShapeExDraw(pShape, 0.5f);
		xgeShapeExDestroy(pShape);

		/* Playhead dot */
		if ( xgeShapeExCreate(&pShape) != XGE_OK ) return;
		{
			float px = barX + barW * progress;
			float py = barY + 16.0f;
			/* Approximate circle with small square */
			xgeShapeExMoveTo(pShape, px - 4, py - 4);
			xgeShapeExLineTo(pShape, px + 4, py - 4);
			xgeShapeExLineTo(pShape, px + 4, py + 4);
			xgeShapeExLineTo(pShape, px - 4, py + 4);
			xgeShapeExClose(pShape);
			xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 255, 255, 255));
			xgeShapeExDraw(pShape, 0.5f);
			xgeShapeExDestroy(pShape);
		}
	}

	/* Play/Pause button indicator */
	if ( xgeShapeExCreate(&pShape) != XGE_OK ) return;
	if ( pApp->bPlaying ) {
		/* Pause icon (two bars) */
		xgeShapeExMoveTo(pShape, 16, barY + 8);
		xgeShapeExLineTo(pShape, 20, barY + 8);
		xgeShapeExLineTo(pShape, 20, barY + 24);
		xgeShapeExLineTo(pShape, 16, barY + 24);
		xgeShapeExClose(pShape);
		xgeShapeExMoveTo(pShape, 24, barY + 8);
		xgeShapeExLineTo(pShape, 28, barY + 8);
		xgeShapeExLineTo(pShape, 28, barY + 24);
		xgeShapeExLineTo(pShape, 24, barY + 24);
		xgeShapeExClose(pShape);
	} else {
		/* Play icon (triangle) */
		xgeShapeExMoveTo(pShape, 16, barY + 8);
		xgeShapeExLineTo(pShape, 30, barY + 16);
		xgeShapeExLineTo(pShape, 16, barY + 24);
		xgeShapeExClose(pShape);
	}
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);
}

/* ------------------------------------------------------------------ */
/* Window title update                                                */
/* ------------------------------------------------------------------ */

void playerShellUpdateTitle(player_app_t* pApp, const char* sBaseTitle)
{
	char title[600];
	const char* fileName;

	/* Extract filename from path */
	fileName = strrchr(pApp->sFilePath, '/');
	if ( fileName == NULL ) fileName = strrchr(pApp->sFilePath, '\\');
	if ( fileName ) fileName++; else fileName = pApp->sFilePath;

	if ( pApp->sFilePath[0] ) {
		sprintf(title, "%s - %s [%d/%d] %.1fx%s",
		        sBaseTitle, fileName,
		        pApp->iCurrentFrame + 1, pApp->iFrameCount,
		        pApp->fSpeed,
		        pApp->bLoop ? " (loop)" : "");
	} else {
		sprintf(title, "%s - no file loaded", sBaseTitle);
	}

	xgeSetTitle(title);
}
