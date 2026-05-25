#include "uid_undo.h"
#include <string.h>

void UIDesignUndoInit(uid_undo_stack_t* pUndo)
{
	if ( pUndo != NULL ) {
		memset(pUndo, 0, sizeof(*pUndo));
	}
}

void UIDesignUndoUnit(uid_undo_stack_t* pUndo)
{
	(void)pUndo;
}
