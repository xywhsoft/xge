/*
 * af_undo.h - AnimForge undo/redo command stack
 *
 * Implements the Command pattern for undo/redo. Each user action
 * is encapsulated as an af_cmd_t with execute/undo function pointers.
 * Commands are pushed onto a stack; undo pops and reverses, redo
 * re-applies.
 */

#ifndef AF_UNDO_H
#define AF_UNDO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AF_UNDO_STACK_MAX   128
#define AF_UNDO_DATA_MAX    4096  /* max bytes per command's private data */

typedef struct af_undo_stack_t af_undo_stack_t;
typedef af_undo_stack_t* af_undo_stack;

/* Command callback: returns 0 on success */
typedef int (*af_cmd_exec_proc)(void* pData, void* pUser);

typedef struct af_cmd_t {
	char     sLabel[64];         /* human-readable description */
	af_cmd_exec_proc pfnExecute; /* execute / redo */
	af_cmd_exec_proc pfnUndo;    /* undo */
	void*    pUser;              /* application context */
	uint8_t  arrData[AF_UNDO_DATA_MAX]; /* command-private snapshot data */
	int      iDataSize;          /* used bytes in arrData */
	uint32_t iGroup;             /* merge group id (0 = no merge) */
} af_cmd_t;

struct af_undo_stack_t {
	af_cmd_t arrCmds[AF_UNDO_STACK_MAX];
	int      iCount;             /* number of commands in stack */
	int      iCursor;            /* current position (commands before cursor are "done") */
	uint32_t iNextGroup;         /* next group id for merging */
	int      bSuspended;         /* when true, execute does not push */
};

/* ------------------------------------------------------------------ */
/* API                                                                */
/* ------------------------------------------------------------------ */

void afUndoInit(af_undo_stack pStack);
void afUndoClear(af_undo_stack pStack);

/* Execute a command and push it onto the stack.
 * Calls pfnExecute immediately, then stores the command for undo. */
int  afUndoExecute(af_undo_stack pStack, const af_cmd_t* pCmd);

/* Undo the last executed command. Returns 0 on success, -1 if nothing to undo. */
int  afUndoUndo(af_undo_stack pStack);

/* Redo the last undone command. Returns 0 on success, -1 if nothing to redo. */
int  afUndoRedo(af_undo_stack pStack);

/* Query state */
int  afUndoCanUndo(af_undo_stack pStack);
int  afUndoCanRedo(af_undo_stack pStack);
int  afUndoGetCount(af_undo_stack pStack);
const char* afUndoGetUndoLabel(af_undo_stack pStack);
const char* afUndoGetRedoLabel(af_undo_stack pStack);

/* Group merging: consecutive commands with the same non-zero group
 * are merged into one undo step. Call afUndoBeginGroup to start. */
uint32_t afUndoBeginGroup(af_undo_stack pStack);
void     afUndoEndGroup(af_undo_stack pStack);

/* Suspend: during programmatic batch changes, suspend undo recording */
void afUndoSuspend(af_undo_stack pStack);
void afUndoResume(af_undo_stack pStack);

#ifdef __cplusplus
}
#endif

#endif /* AF_UNDO_H */
