/*
 * af_tool_text.c - Text tool
 *
 * Creates and edits text objects on the stage.
 */

#include "af_tools.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ------------------------------------------------------------------ */
/* Text tool state                                                    */
/* ------------------------------------------------------------------ */

#define AF_TEXT_MAX_CHARS  1024

typedef struct af_text_tool_state_t {
	int   bEditing;
	float fX, fY;
	char  sText[AF_TEXT_MAX_CHARS];
	int   iCursorPos;
	int   iFontSize;
	uint32_t iColor;
} af_text_tool_state_t;

/* ------------------------------------------------------------------ */
/* Handlers                                                           */
/* ------------------------------------------------------------------ */

static int __textActivate(af_tool pTool, af_tool_context_t* pCtx)
{
	af_text_tool_state_t* pState = (af_text_tool_state_t*)pTool->pState;
	(void)pCtx;
	if ( pState ) {
		pState->bEditing = 0;
		pState->iFontSize = 16;
		pState->iColor = 0x000000FF;
		pState->sText[0] = '\0';
		pState->iCursorPos = 0;
	}
	return 0;
}

static int __textDeactivate(af_tool pTool, af_tool_context_t* pCtx)
{
	af_text_tool_state_t* pState = (af_text_tool_state_t*)pTool->pState;
	(void)pCtx;
	if ( pState )
		pState->bEditing = 0;
	return 0;
}

static int __textMouseDown(af_tool pTool, af_tool_context_t* pCtx, float fX, float fY, uint32_t iButton)
{
	af_text_tool_state_t* pState = (af_text_tool_state_t*)pTool->pState;
	(void)pCtx;

	if ( iButton != XGE_MOUSE_LEFT ) return 0;
	if ( pState == NULL ) return -1;

	/* Start new text at click position */
	pState->bEditing = 1;
	pState->fX = fX;
	pState->fY = fY;
	pState->sText[0] = '\0';
	pState->iCursorPos = 0;
	pState->iColor = pCtx->iFillColor;

	return 0;
}

static int __textMouseMove(af_tool pTool, af_tool_context_t* pCtx, float fX, float fY)
{
	(void)pTool; (void)pCtx; (void)fX; (void)fY;
	return 0;
}

static int __textMouseUp(af_tool pTool, af_tool_context_t* pCtx, float fX, float fY, uint32_t iButton)
{
	(void)pTool; (void)pCtx; (void)fX; (void)fY; (void)iButton;
	return 0;
}

static int __textKeyDown(af_tool pTool, af_tool_context_t* pCtx, int iKey)
{
	af_text_tool_state_t* pState = (af_text_tool_state_t*)pTool->pState;
	int len;
	(void)pCtx;

	if ( pState == NULL || !pState->bEditing )
		return 0;

	len = (int)strlen(pState->sText);

	/* Backspace */
	if ( iKey == 259 ) {
		if ( pState->iCursorPos > 0 ) {
			memmove(&pState->sText[pState->iCursorPos - 1],
			        &pState->sText[pState->iCursorPos],
			        len - pState->iCursorPos + 1);
			pState->iCursorPos--;
		}
		return 1;
	}

	/* Delete */
	if ( iKey == 261 ) {
		if ( pState->iCursorPos < len ) {
			memmove(&pState->sText[pState->iCursorPos],
			        &pState->sText[pState->iCursorPos + 1],
			        len - pState->iCursorPos);
		}
		return 1;
	}

	/* Left arrow */
	if ( iKey == 263 ) {
		if ( pState->iCursorPos > 0 )
			pState->iCursorPos--;
		return 1;
	}

	/* Right arrow */
	if ( iKey == 262 ) {
		if ( pState->iCursorPos < len )
			pState->iCursorPos++;
		return 1;
	}

	/* Enter - commit */
	if ( iKey == 257 ) {
		pState->bEditing = 0;
		return 1;
	}

	/* Escape - cancel */
	if ( iKey == 256 ) {
		pState->sText[0] = '\0';
		pState->bEditing = 0;
		return 1;
	}

	/* Printable characters */
	if ( iKey >= 32 && iKey <= 126 ) {
		if ( len < AF_TEXT_MAX_CHARS - 2 ) {
			memmove(&pState->sText[pState->iCursorPos + 1],
			        &pState->sText[pState->iCursorPos],
			        len - pState->iCursorPos + 1);
			pState->sText[pState->iCursorPos] = (char)iKey;
			pState->iCursorPos++;
		}
		return 1;
	}

	return 0;
}

static int __textRender(af_tool pTool, af_tool_context_t* pCtx)
{
	af_text_tool_state_t* pState = (af_text_tool_state_t*)pTool->pState;
	(void)pCtx;

	if ( pState == NULL || !pState->bEditing )
		return 0;

	/* Text preview rendering would go here using ShapeEx or proxy draw */
	return 0;
}

static int __textCancel(af_tool pTool, af_tool_context_t* pCtx)
{
	af_text_tool_state_t* pState = (af_text_tool_state_t*)pTool->pState;
	(void)pCtx;
	if ( pState ) {
		pState->bEditing = 0;
		pState->sText[0] = '\0';
	}
	return 0;
}

/* ------------------------------------------------------------------ */
/* Creation                                                           */
/* ------------------------------------------------------------------ */

af_tool afToolTextCreate(void)
{
	af_tool pTool;
	af_text_tool_state_t* pState;

	pTool = (af_tool)calloc(1, sizeof(af_tool_t));
	if ( pTool == NULL ) return NULL;

	pState = (af_text_tool_state_t*)calloc(1, sizeof(af_text_tool_state_t));
	if ( pState == NULL ) {
		free(pTool);
		return NULL;
	}

	pTool->iId = 10;  /* AF_TOOL_TEXT */
	pTool->sName = "Text";
	pTool->sShortcut = "T";
	pTool->iCursor = 0;

	pTool->pfnActivate = __textActivate;
	pTool->pfnDeactivate = __textDeactivate;
	pTool->pfnMouseDown = __textMouseDown;
	pTool->pfnMouseMove = __textMouseMove;
	pTool->pfnMouseUp = __textMouseUp;
	pTool->pfnKeyDown = __textKeyDown;
	pTool->pfnRender = __textRender;
	pTool->pfnCancel = __textCancel;

	pTool->pState = pState;
	pState->iFontSize = 16;
	pState->iColor = 0x000000FF;

	return pTool;
}
