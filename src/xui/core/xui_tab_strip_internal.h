#ifndef XUI_TAB_STRIP_INTERNAL_H
#define XUI_TAB_STRIP_INTERNAL_H

typedef struct xui_internal_tab_strip_window_t {
	int iFirst;
	int iVisibleCount;
	int iMaxFirst;
	int bOverflow;
	int bShowBackward;
	int bShowForward;
} xui_internal_tab_strip_window_t;

static float xuiInternalTabStripUsedAxis(const float* arrWidths, int iFirst,
	int iCount, float fOverlap)
{
	float fAxis;
	int i;

	if ( arrWidths == NULL || iFirst < 0 || iCount <= 0 ) return 0.0f;
	fAxis = arrWidths[iFirst];
	for ( i = 1; i < iCount; ++i ) {
		float fStep = arrWidths[iFirst + i] - fOverlap;
		fAxis += fStep > 0.0f ? fStep : 0.0f;
	}
	return fAxis;
}

static int xuiInternalTabStripFitForward(const float* arrWidths, int iItemCount,
	int iFirst, float fAvailableAxis, float fOverlap)
{
	float fUsed;
	int iCount;

	if ( arrWidths == NULL || iItemCount <= 0 || iFirst < 0 ||
	     iFirst >= iItemCount || fAvailableAxis <= 0.0f ) return 0;
	fUsed = 0.0f;
	iCount = 0;
	while ( iFirst + iCount < iItemCount ) {
		float fAdd = arrWidths[iFirst + iCount];
		if ( iCount > 0 ) {
			fAdd -= fOverlap;
			if ( fAdd < 0.0f ) fAdd = 0.0f;
		}
		if ( iCount > 0 && fUsed + fAdd > fAvailableAxis + 0.5f ) break;
		fUsed += fAdd;
		iCount++;
		if ( fUsed > fAvailableAxis + 0.5f ) break;
	}
	return iCount;
}

static int xuiInternalTabStripMaxFirst(const float* arrWidths, int iItemCount,
	float fAvailableAxis, float fOverlap)
{
	float fUsed;
	int iFirst;

	if ( arrWidths == NULL || iItemCount <= 0 ) return 0;
	if ( fAvailableAxis <= 0.0f ) return iItemCount - 1;
	iFirst = iItemCount - 1;
	fUsed = arrWidths[iFirst];
	while ( iFirst > 0 ) {
		float fAdd = arrWidths[iFirst - 1] - fOverlap;
		if ( fAdd < 0.0f ) fAdd = 0.0f;
		if ( fUsed + fAdd > fAvailableAxis + 0.5f ) break;
		fUsed += fAdd;
		iFirst--;
	}
	return iFirst;
}

static xui_internal_tab_strip_window_t xuiInternalTabStripResolve(
	const float* arrWidths, int iItemCount, int iRequestedFirst,
	float fAvailableAxis, float fOverlap, int bScrollButtons,
	float fButtonAxis, float fButtonGap)
{
	xui_internal_tab_strip_window_t tWindow;
	float fControlAxis;
	float fItemsAxis;
	int iVisible;

	tWindow.iFirst = 0;
	tWindow.iVisibleCount = 0;
	tWindow.iMaxFirst = 0;
	tWindow.bOverflow = 0;
	tWindow.bShowBackward = 0;
	tWindow.bShowForward = 0;
	if ( arrWidths == NULL || iItemCount <= 0 || fAvailableAxis <= 0.0f ) {
		return tWindow;
	}
	fItemsAxis = xuiInternalTabStripUsedAxis(arrWidths, 0, iItemCount, fOverlap);
	if ( fItemsAxis <= fAvailableAxis + 0.5f ) {
		tWindow.iVisibleCount = iItemCount;
		return tWindow;
	}
	tWindow.bOverflow = 1;
	fControlAxis = bScrollButtons ? fButtonAxis + fButtonGap : 0.0f;
	tWindow.iMaxFirst = xuiInternalTabStripMaxFirst(arrWidths, iItemCount,
		fAvailableAxis - fControlAxis, fOverlap);
	if ( iRequestedFirst < 0 ) iRequestedFirst = 0;
	if ( iRequestedFirst > tWindow.iMaxFirst ) iRequestedFirst = tWindow.iMaxFirst;
	tWindow.iFirst = iRequestedFirst;
	tWindow.bShowBackward = bScrollButtons && tWindow.iFirst > 0;
	fItemsAxis = fAvailableAxis - (tWindow.bShowBackward ? fControlAxis : 0.0f);
	iVisible = xuiInternalTabStripFitForward(arrWidths, iItemCount,
		tWindow.iFirst, fItemsAxis, fOverlap);
	tWindow.bShowForward = bScrollButtons &&
		(tWindow.iFirst + iVisible < iItemCount);
	if ( tWindow.bShowForward ) {
		fItemsAxis -= fControlAxis;
		iVisible = xuiInternalTabStripFitForward(arrWidths, iItemCount,
			tWindow.iFirst, fItemsAxis, fOverlap);
	}
	if ( iVisible < 1 ) iVisible = 1;
	if ( iVisible > iItemCount - tWindow.iFirst ) {
		iVisible = iItemCount - tWindow.iFirst;
	}
	tWindow.iVisibleCount = iVisible;
	return tWindow;
}

#endif
