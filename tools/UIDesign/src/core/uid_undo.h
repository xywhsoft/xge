#ifndef UID_UNDO_H
#define UID_UNDO_H

#include "xge.h"

typedef struct uid_undo_stack_t {
	int iUndoCount;
	int iRedoCount;
} uid_undo_stack_t;

void UIDesignUndoInit(uid_undo_stack_t* pUndo);
void UIDesignUndoUnit(uid_undo_stack_t* pUndo);

#endif
