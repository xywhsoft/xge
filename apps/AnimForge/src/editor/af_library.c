/*
 * af_library.c - Library panel
 *
 * Manages the Symbol/Resource library UI: listing, search,
 * drag-to-stage, context menu operations.
 */

#include "af_library.h"
#include "af_draw_helper.h"
#include "../core/af_app.h"
#include "../anim/af_symbol.h"
#include <string.h>
#include <stdio.h>

/* ------------------------------------------------------------------ */
/* State                                                              */
/* ------------------------------------------------------------------ */

typedef struct af_library_state_t {
	char     sFilter[128];       /* search filter text */
	int      iFilterType;        /* -1=all, 0=graphic, 1=sprite, 2=group, 3=movieclip */
	int      iSelectedIndex;     /* selected item in list */
	int      iScrollOffset;      /* scroll position */
	uint32_t iSelectedSymbolId;  /* selected symbol ID */
	int      bDragging;          /* drag-to-stage in progress */
	float    fDragX, fDragY;
} af_library_state_t;

static af_library_state_t s_tLibState;

/* ------------------------------------------------------------------ */
/* Init / Shutdown                                                    */
/* ------------------------------------------------------------------ */

void afLibraryInit(af_library_t* pLib)
{
	memset(pLib, 0, sizeof(*pLib));
	memset(&s_tLibState, 0, sizeof(s_tLibState));
	s_tLibState.iFilterType = -1;
	s_tLibState.iSelectedIndex = -1;
}

void afLibraryShutdown(af_library_t* pLib)
{
	(void)pLib;
}

/* ------------------------------------------------------------------ */
/* Filtering                                                          */
/* ------------------------------------------------------------------ */

static int __afLibraryMatchFilter(af_symbol_t* pSym)
{
	/* Type filter */
	if ( s_tLibState.iFilterType >= 0 ) {
		if ( pSym->iType != s_tLibState.iFilterType + 1 )
			return 0;
	}

	/* Text filter */
	if ( s_tLibState.sFilter[0] != '\0' ) {
		/* Simple case-insensitive substring match */
		const char* name = pSym->sName;
		const char* filter = s_tLibState.sFilter;
		int nameLen = (int)strlen(name);
		int filterLen = (int)strlen(filter);
		int i, j;

		for ( i = 0; i <= nameLen - filterLen; i++ ) {
			int match = 1;
			for ( j = 0; j < filterLen; j++ ) {
				char a = name[i + j];
				char b = filter[j];
				if ( a >= 'A' && a <= 'Z' ) a += 32;
				if ( b >= 'A' && b <= 'Z' ) b += 32;
				if ( a != b ) { match = 0; break; }
			}
			if ( match ) return 1;
		}
		return 0;
	}

	return 1;
}

/* ------------------------------------------------------------------ */
/* Rendering                                                          */
/* ------------------------------------------------------------------ */

void afLibraryRender(af_library_t* pLib, xui_proxy_t* pProxy, xui_draw_context pCtx,
                     float fW, float fH, af_doc pDoc)
{
	float itemH = 24.0f;
	float y = 30.0f;
	int i;
	uint32_t bgColor = AF_COLOR_PANEL;
	uint32_t selColor = AF_COLOR_HIGHLIGHT;
	uint32_t textColor = AF_COLOR_TEXT;
	uint32_t dimColor = AF_COLOR_TEXT_DIM;

	/* Background */
	afDrawFillRect(pProxy, pCtx, 0, 0, fW, fH, bgColor);

	/* Search box area */
	afDrawFillRect(pProxy, pCtx, 4, 4, fW - 8, 22, XUI_COLOR_RGBA(60, 60, 64, 255));

	/* Filter type buttons */
	{
		float bx = 4;
		int t;
		for ( t = 0; t < 5; t++ ) {
			float bw = 30.0f;
			uint32_t btnColor = (s_tLibState.iFilterType == t - 1) ?
			                    AF_COLOR_ACCENT : XUI_COLOR_RGBA(50, 50, 54, 255);
			afDrawFillRect(pProxy, pCtx, bx, fH - 24, bw, 20, btnColor);
			bx += bw + 2;
		}
	}

	/* Symbol list */
	for ( i = 0; i < pDoc->iSymbolCount; i++ ) {
		af_symbol_t* pSym = &pDoc->arrSymbols[i];
		uint32_t iconColor;

		if ( !__afLibraryMatchFilter(pSym) )
			continue;

		if ( y + itemH > fH - 28 )
			break;

		/* Selection highlight */
		if ( i == s_tLibState.iSelectedIndex ) {
			afDrawFillRect(pProxy, pCtx, 2, y, fW - 4, itemH, selColor);
		}

		/* Type icon (colored square) */
		switch ( pSym->iType ) {
		case AF_SYMBOL_GRAPHIC:    iconColor = XUI_COLOR_RGBA(100, 200, 100, 255); break;
		case AF_SYMBOL_SPRITE:     iconColor = XUI_COLOR_RGBA(100, 150, 255, 255); break;
		case AF_SYMBOL_GROUP:      iconColor = XUI_COLOR_RGBA(255, 200, 100, 255); break;
		case AF_SYMBOL_MOVIE_CLIP: iconColor = XUI_COLOR_RGBA(255, 100, 150, 255); break;
		default:                   iconColor = dimColor; break;
		}

		afDrawFillRect(pProxy, pCtx, 6, y + 4, 12, 12, iconColor);

		/* Name text placeholder */
		afDrawFillRect(pProxy, pCtx, 24, y + 8, (float)(strlen(pSym->sName) * 6), 8, textColor);

		y += itemH;
	}

	/* Resource section header */
	y += 8;
	afDrawFillRect(pProxy, pCtx, 4, y, fW - 8, 1, dimColor);
	y += 4;

	/* Resource list */
	for ( i = 0; i < pDoc->iResourceCount; i++ ) {
		af_resource_t* pRes = &pDoc->arrResources[i];
		uint32_t iconColor;

		if ( y + itemH > fH - 28 )
			break;

		switch ( pRes->iType ) {
		case AF_RES_TEXTURE: iconColor = XUI_COLOR_RGBA(150, 100, 255, 255); break;
		case AF_RES_FONT:    iconColor = XUI_COLOR_RGBA(255, 255, 100, 255); break;
		case AF_RES_AUDIO:   iconColor = XUI_COLOR_RGBA(100, 255, 200, 255); break;
		default:             iconColor = dimColor; break;
		}

		afDrawFillRect(pProxy, pCtx, 6, y + 4, 12, 12, iconColor);
		afDrawFillRect(pProxy, pCtx, 24, y + 8, (float)(strlen(pRes->sName) * 6), 8, textColor);

		y += itemH;
	}
}

/* ------------------------------------------------------------------ */
/* Input                                                              */
/* ------------------------------------------------------------------ */

int afLibraryMouseDown(af_library_t* pLib, float fX, float fY,
                       float fW, float fH, af_doc pDoc)
{
	float itemH = 24.0f;
	float y = 30.0f;
	int i, visIdx = 0;

	/* Filter type buttons */
	if ( fY >= fH - 24 && fY <= fH - 4 ) {
		int btn = (int)(fX / 32.0f);
		if ( btn >= 0 && btn < 5 ) {
			s_tLibState.iFilterType = btn - 1;
			return 1;
		}
	}

	/* Symbol list hit test */
	for ( i = 0; i < pDoc->iSymbolCount; i++ ) {
		if ( !__afLibraryMatchFilter(&pDoc->arrSymbols[i]) )
			continue;

		if ( fY >= y && fY < y + itemH ) {
			s_tLibState.iSelectedIndex = i;
			s_tLibState.iSelectedSymbolId = pDoc->arrSymbols[i].iId;
			return 1;
		}
		y += itemH;
		visIdx++;
	}

	s_tLibState.iSelectedIndex = -1;
	s_tLibState.iSelectedSymbolId = 0;
	(void)visIdx;
	return 0;
}

/* ------------------------------------------------------------------ */
/* Operations                                                         */
/* ------------------------------------------------------------------ */

int afLibraryInstantiateSelected(af_library_t* pLib, af_doc pDoc,
                                 uint32_t iLayerId, uint32_t iFrame,
                                 float fX, float fY)
{
	if ( s_tLibState.iSelectedSymbolId == 0 )
		return -1;

	return afSymbolInstantiate(pDoc, s_tLibState.iSelectedSymbolId,
	                           iLayerId, iFrame, fX, fY, NULL);
}

int afLibraryDeleteSelected(af_library_t* pLib, af_doc pDoc)
{
	if ( s_tLibState.iSelectedSymbolId == 0 )
		return -1;

	/* Check usage */
	if ( afSymbolGetUsageCount(pDoc, s_tLibState.iSelectedSymbolId) > 0 )
		return -2; /* in use */

	afDocRemoveSymbol(pDoc, s_tLibState.iSelectedSymbolId);
	s_tLibState.iSelectedSymbolId = 0;
	s_tLibState.iSelectedIndex = -1;
	return 0;
}

int afLibraryDuplicateSelected(af_library_t* pLib, af_doc pDoc)
{
	af_symbol_t* pSym;
	char newName[AF_DOC_MAX_NAME];

	if ( s_tLibState.iSelectedSymbolId == 0 )
		return -1;

	pSym = afDocFindSymbol(pDoc, s_tLibState.iSelectedSymbolId);
	if ( pSym == NULL )
		return -1;

	sprintf(newName, "%s_copy", pSym->sName);
	return afSymbolDuplicate(pDoc, s_tLibState.iSelectedSymbolId, newName, NULL);
}

void afLibrarySetFilter(af_library_t* pLib, const char* sFilter)
{
	if ( sFilter ) {
		strncpy(s_tLibState.sFilter, sFilter, sizeof(s_tLibState.sFilter) - 1);
		s_tLibState.sFilter[sizeof(s_tLibState.sFilter) - 1] = '\0';
	} else {
		s_tLibState.sFilter[0] = '\0';
	}
}

uint32_t afLibraryGetSelectedSymbol(af_library_t* pLib)
{
	return s_tLibState.iSelectedSymbolId;
}
