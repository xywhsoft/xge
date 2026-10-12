/*
 * af_library.h - Library panel
 */

#ifndef AF_LIBRARY_H
#define AF_LIBRARY_H

#include "xui.h"
#include "../core/af_doc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct af_library_t {
	xui_widget pPanel;
	xui_widget pSearchInput;
	xui_widget pListView;
} af_library_t;

void afLibraryInit(af_library_t* pLib);
void afLibraryShutdown(af_library_t* pLib);

void afLibraryRender(af_library_t* pLib, xui_proxy_t* pProxy, xui_draw_context pCtx,
                     float fW, float fH, af_doc pDoc);
int afLibraryMouseDown(af_library_t* pLib, float fX, float fY,
                       float fW, float fH, af_doc pDoc);

int afLibraryInstantiateSelected(af_library_t* pLib, af_doc pDoc,
                                 uint32_t iLayerId, uint32_t iFrame,
                                 float fX, float fY);
int afLibraryDeleteSelected(af_library_t* pLib, af_doc pDoc);
int afLibraryDuplicateSelected(af_library_t* pLib, af_doc pDoc);
void afLibrarySetFilter(af_library_t* pLib, const char* sFilter);
uint32_t afLibraryGetSelectedSymbol(af_library_t* pLib);

#ifdef __cplusplus
}
#endif

#endif /* AF_LIBRARY_H */
