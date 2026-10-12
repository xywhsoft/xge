#include "ui_design_session.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(expression) do { if (!(expression)) { fprintf(stderr, "session-test failed at %d: %s\n", __LINE__, #expression); return 1; } } while (0)

static char* copyText(const char* text)
{
	char* result = (char*)malloc(strlen(text) + 1);
	if ( result ) strcpy(result, text);
	return result;
}

int main(int argc, char** argv)
{
	ui_design_session_t* a = uiDesignSessionCreate(1);
	ui_design_session_t* b = uiDesignSessionCreate(2);
	ui_design_history_entry_t entry = {0};
	ui_design_model_t* loaded = NULL;
	ui_design_file_stamp_t stamp;
	char path[UI_DESIGN_PATH_CAPACITY], alias[UI_DESIGN_PATH_CAPACITY], invalid[UI_DESIGN_PATH_CAPACITY];
	char* saved = NULL;
	char* text;
	uint64_t revision;
	int i, id;
	CHECK(argc == 2 && a && b);
	CHECK(!a->bDocumentDirty && a->fZoom == 1 && uiDesignSessionHistoryBytes(a) == 0);
	CHECK(uiDesignModelAddNode(&a->tModel, UI_DESIGN_NODE_BUTTON, 0, 20, 30, &id) == XUI_OK);
	CHECK(uiDesignSessionUpdateDirty(a) == XUI_OK && a->bDocumentDirty);
	CHECK(uiDesignSessionSetClean(a) == XUI_OK && !a->bDocumentDirty);
	CHECK(uiDesignDocumentSaveContent(&a->tModel, &saved) == XUI_OK);
	CHECK(uiDesignModelSetSelected(&a->tModel, id) == XUI_OK);
	CHECK(uiDesignSessionUpdateDirty(a) == XUI_OK && !a->bDocumentDirty);
	CHECK(uiDesignDocumentLoadModel(saved, &loaded) == XUI_OK && loaded->iSelectedCount == 0);
	uiDesignModelDestroy(loaded); free(loaded);
	revision = a->iRevision;
	CHECK(uiDesignSessionAcceptResult(a, 1, revision));
	CHECK(!uiDesignSessionAcceptResult(b, 1, revision));
	CHECK(uiDesignModelSetText(&a->tModel, id, "清晰的中文 / Clear text") == XUI_OK);
	CHECK(uiDesignSessionUpdateDirty(a) == XUI_OK && a->bDocumentDirty);
	CHECK(!uiDesignSessionAcceptResult(a, 1, revision));
	CHECK(b->tModel.iNodeCount == 0 && !b->bDocumentDirty);
	for ( i = 0; i < 70; ++i ) {
		entry.sBefore = copyText("before");
		entry.sAfter = copyText("after");
		entry.iCommand = i;
		CHECK(entry.sBefore && entry.sAfter);
		uiDesignHistoryPush(a->arrUndo, &a->iUndoCount, &entry);
	}
	CHECK(a->iUndoCount == 64 && a->arrUndo[0].iCommand == 6);
	CHECK(uiDesignSessionHistoryBytes(a) == 64 * 13);
	uiDesignSessionTrimHistory(a, 26);
	CHECK(a->iUndoCount == 2 && a->arrUndo[0].iCommand == 68);
	entry = a->arrUndo[--a->iUndoCount];
	memset(&a->arrUndo[a->iUndoCount], 0, sizeof(entry));
	uiDesignHistoryPush(a->arrRedo, &a->iRedoCount, &entry);
	CHECK(uiDesignSessionHistoryBytes(a) == 26);
	uiDesignSessionTrimHistory(a, 13);
	CHECK(a->iRedoCount == 0 && a->iUndoCount == 1);
	uiDesignSessionTrimHistory(a, 0);
	CHECK(a->iUndoCount == 0 && uiDesignSessionHistoryBytes(a) == 0);
	snprintf(path, sizeof(path), "%s/中文文档.json", argv[1]);
	snprintf(alias, sizeof(alias), "%s/./中文文档.json", argv[1]);
	snprintf(invalid, sizeof(invalid), "%s/no-such-directory/document.json", argv[1]);
	CHECK(uiDesignFileWrite(path, saved) == XUI_OK);
	CHECK(uiDesignFileSamePath(path, alias));
	stamp = uiDesignFileStamp(path);
	CHECK(stamp.bPresent);
	text = uiDesignFileRead(path, UI_DESIGN_DOCUMENT_MAX_BYTES);
	CHECK(text && strcmp(text, saved) == 0); free(text);
	CHECK(uiDesignFileRead(path, 5) == NULL);
	CHECK(uiDesignFileWrite(invalid, "do not replace the original") != XUI_OK);
	CHECK(uiDesignFileStampEqual(stamp, uiDesignFileStamp(path)));
	CHECK(uiDesignFileWrite(path, "externally changed") == XUI_OK);
	CHECK(!uiDesignFileStampEqual(stamp, uiDesignFileStamp(path)));
	CHECK(xrtFileWriteAtomic(path, (xbytesview){(const unsigned char*)"a\0b", 3}));
	CHECK(uiDesignFileRead(path, 100) == NULL);
	free(saved);
	uiDesignSessionDestroy(a); uiDesignSessionDestroy(b);
	puts("uidesign-session-tests: dirty/selection, isolation, revisions, history budgets, UTF-8 and atomic IO passed");
	return 0;
}
