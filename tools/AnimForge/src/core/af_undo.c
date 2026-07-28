/*
 * af_undo.c - AnimForge undo/redo command stack implementation
 */

#include "af_undo.h"
#include <string.h>

void afUndoInit(af_undo_stack pStack)
{
	if ( pStack == NULL ) return;
	memset(pStack, 0, sizeof(af_undo_stack_t));
	pStack->iNextGroup = 1;
}

void afUndoClear(af_undo_stack pStack)
{
	if ( pStack == NULL ) return;
	pStack->iCount = 0;
	pStack->iCursor = 0;
}

int afUndoExecute(af_undo_stack pStack, const af_cmd_t* pCmd)
{
	int ret;
	af_cmd_t* pSlot;
	if ( pStack == NULL || pCmd == NULL ) return -1;

	/* Execute the command immediately */
	if ( pCmd->pfnExecute != NULL ) {
		ret = pCmd->pfnExecute((void*)pCmd->arrData, pCmd->pUser);
		if ( ret != 0 ) return ret;
	}

	/* If suspended, don't record */
	if ( pStack->bSuspended ) return 0;

	/* Truncate any redo history beyond cursor */
	pStack->iCount = pStack->iCursor;

	/* Check stack capacity */
	if ( pStack->iCount >= AF_UNDO_STACK_MAX ) {
		/* Shift stack down by one (discard oldest) */
		memmove(&pStack->arrCmds[0], &pStack->arrCmds[1],
			(size_t)(AF_UNDO_STACK_MAX - 1) * sizeof(af_cmd_t));
		pStack->iCount = AF_UNDO_STACK_MAX - 1;
		pStack->iCursor = pStack->iCount;
	}

	/* Store command */
	pSlot = &pStack->arrCmds[pStack->iCount];
	memcpy(pSlot, pCmd, sizeof(af_cmd_t));
	pStack->iCount++;
	pStack->iCursor = pStack->iCount;
	return 0;
}

int afUndoUndo(af_undo_stack pStack)
{
	af_cmd_t* pCmd;
	if ( pStack == NULL ) return -1;
	if ( pStack->iCursor <= 0 ) return -1; /* nothing to undo */

	pStack->iCursor--;
	pCmd = &pStack->arrCmds[pStack->iCursor];

	if ( pCmd->pfnUndo != NULL ) {
		return pCmd->pfnUndo((void*)pCmd->arrData, pCmd->pUser);
	}
	return 0;
}

int afUndoRedo(af_undo_stack pStack)
{
	af_cmd_t* pCmd;
	if ( pStack == NULL ) return -1;
	if ( pStack->iCursor >= pStack->iCount ) return -1; /* nothing to redo */

	pCmd = &pStack->arrCmds[pStack->iCursor];
	pStack->iCursor++;

	if ( pCmd->pfnExecute != NULL ) {
		return pCmd->pfnExecute((void*)pCmd->arrData, pCmd->pUser);
	}
	return 0;
}

int afUndoCanUndo(af_undo_stack pStack)
{
	if ( pStack == NULL ) return 0;
	return pStack->iCursor > 0;
}

int afUndoCanRedo(af_undo_stack pStack)
{
	if ( pStack == NULL ) return 0;
	return pStack->iCursor < pStack->iCount;
}

int afUndoGetCount(af_undo_stack pStack)
{
	if ( pStack == NULL ) return 0;
	return pStack->iCount;
}

const char* afUndoGetUndoLabel(af_undo_stack pStack)
{
	if ( pStack == NULL ) return NULL;
	if ( pStack->iCursor <= 0 ) return NULL;
	return pStack->arrCmds[pStack->iCursor - 1].sLabel;
}

const char* afUndoGetRedoLabel(af_undo_stack pStack)
{
	if ( pStack == NULL ) return NULL;
	if ( pStack->iCursor >= pStack->iCount ) return NULL;
	return pStack->arrCmds[pStack->iCursor].sLabel;
}

uint32_t afUndoBeginGroup(af_undo_stack pStack)
{
	if ( pStack == NULL ) return 0;
	return pStack->iNextGroup++;
}

void afUndoEndGroup(af_undo_stack pStack)
{
	(void)pStack;
	/* Group merging is handled by comparing iGroup in afUndoExecute.
	 * For now, groups are simply marked; full merge logic is a
	 * future enhancement. */
}

void afUndoSuspend(af_undo_stack pStack)
{
	if ( pStack == NULL ) return;
	pStack->bSuspended = 1;
}

void afUndoResume(af_undo_stack pStack)
{
	if ( pStack == NULL ) return;
	pStack->bSuspended = 0;
}
