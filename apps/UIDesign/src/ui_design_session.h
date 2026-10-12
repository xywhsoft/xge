#ifndef UI_DESIGN_SESSION_H
#define UI_DESIGN_SESSION_H

#include "ui_design_document.h"
#include <stdint.h>

#define UI_DESIGN_HISTORY_LIMIT 64
#define UI_DESIGN_HISTORY_BYTES (64u * 1024u * 1024u)
#define UI_DESIGN_TOTAL_HISTORY_BYTES (128u * 1024u * 1024u)
#define UI_DESIGN_SESSION_LIMIT 16
#define UI_DESIGN_PATH_CAPACITY 520

typedef struct ui_design_history_entry_t {
	int iCommand;
	char sName[48];
	char* sBefore;
	char* sAfter;
	size_t iBytes;
} ui_design_history_entry_t;

typedef struct ui_design_file_stamp_t {
	uint64_t iTime;
	uint64_t iSize;
	int bPresent;
} ui_design_file_stamp_t;

/* One design document. The workbench owns widgets; inactive sessions retain
 * their model, history and view state, with no live widget resources. */
typedef struct ui_design_session_t {
	uint64_t iId;
	uint64_t iRevision;
	ui_design_model_t tModel;
	ui_design_history_entry_t arrUndo[UI_DESIGN_HISTORY_LIMIT];
	ui_design_history_entry_t arrRedo[UI_DESIGN_HISTORY_LIMIT];
	int iUndoCount;
	int iRedoCount;
	size_t iHistoryBudget;
	char* sHistoryTransactionBefore;
	int iHistoryTransactionCommand;
	char sHistoryTransactionName[48];
	char sDocumentPath[UI_DESIGN_PATH_CAPACITY];
	char* sCleanSnapshot;
	int bDocumentDirty;
	float fZoom;
	float fCanvasPanX;
	float fCanvasPanY;
	int bGridVisible;
	int bSnapEnabled;
	int bMarqueeSelectContain;
	ui_design_file_stamp_t tDisk;
	int bDiskKnown;
	int bExternalChange;
	int bCloseApproved;
} ui_design_session_t;

ui_design_session_t* uiDesignSessionCreate(uint64_t iId);
void uiDesignSessionDestroy(ui_design_session_t* pSession);
int uiDesignSessionSetClean(ui_design_session_t* pSession);
int uiDesignSessionUpdateDirty(ui_design_session_t* pSession);
int uiDesignSessionAcceptResult(const ui_design_session_t* pSession, uint64_t iId, uint64_t iRevision);
void uiDesignHistoryEntryFree(ui_design_history_entry_t* pEntry);
void uiDesignHistoryClear(ui_design_history_entry_t* pEntries, int* pCount);
void uiDesignHistoryPush(ui_design_history_entry_t* pEntries, int* pCount, ui_design_history_entry_t* pEntry);
size_t uiDesignSessionHistoryBytes(const ui_design_session_t* pSession);
void uiDesignSessionTrimHistory(ui_design_session_t* pSession, size_t iBudget);
char* uiDesignFileRead(const char* sPath, size_t iLimit);
int uiDesignFileWrite(const char* sPath, const char* sText);
int uiDesignFileCanonicalPath(const char* sPath, char* sOut, size_t iCapacity);
int uiDesignFileSamePath(const char* sA, const char* sB);
ui_design_file_stamp_t uiDesignFileStamp(const char* sPath);
int uiDesignFileStampEqual(ui_design_file_stamp_t tA, ui_design_file_stamp_t tB);

#endif
