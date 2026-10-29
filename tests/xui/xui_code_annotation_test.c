#include "xui.h"

#include <stdio.h>
#include <string.h>

static int g_iAnnotationFailAllocation = -1;
static int g_bAnnotationAllocationFailed = 0;

static void* xuiTestAnnotationMalloc(size_t iSize)
{
	if ( g_iAnnotationFailAllocation >= 0 && g_iAnnotationFailAllocation-- == 0 ) {
		g_bAnnotationAllocationFailed = 1;
		return NULL;
	}
	return xrtMalloc(iSize);
}

static void* xuiTestAnnotationRealloc(void* pMemory, size_t iSize)
{
	if ( g_iAnnotationFailAllocation >= 0 && g_iAnnotationFailAllocation-- == 0 ) {
		g_bAnnotationAllocationFailed = 1;
		return NULL;
	}
	return xrtRealloc(pMemory, iSize);
}

#define xrtMalloc xuiTestAnnotationMalloc
#define xrtRealloc xuiTestAnnotationRealloc
#include "../../src/xui/code/xui_code_annotation.c"
#undef xrtMalloc
#undef xrtRealloc

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_code_annotation_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

int main(void)
{
	xui_code_annotation_store pStore;
	xui_code_annotation_store pAuxStore;
	xui_code_marker_t arrMarkers[4];
	xui_code_indicator_t arrIndicators[4];
	xui_code_diagnostic_t arrDiagnostics[2];
	xui_code_diagnostic_t arrReplacement[2];
	xui_code_diagnostic_t tDiagnostic;
	int arrDiagnosticIndices[2];
	uint32_t iDiagnosticVersion;
	int iCount;
	int i;
	int iFail;
	int iFailed;
	int iRet;
	int bReplacementSucceeded;

	pStore = NULL;
	pAuxStore = NULL;
	iFailed = 0;

	iRet = xuiCodeAnnotationStoreCreate(&pStore);
	XUI_TEST_CHECK(iRet == XUI_OK && pStore != NULL, "store create");

	iRet = xuiCodeAnnotationSetMarker(pStore, 3, XUI_CODE_MARKER_BREAKPOINT, 7u, "break", 11u);
	XUI_TEST_CHECK(iRet == XUI_OK, "set marker");
	iRet = xuiCodeAnnotationSetMarker(pStore, 3, XUI_CODE_MARKER_BOOKMARK, 0u, NULL, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK, "set second marker");
	iRet = xuiCodeAnnotationSetMarker(pStore, 3, XUI_CODE_MARKER_BREAKPOINT, 9u, "updated", 12u);
	XUI_TEST_CHECK(iRet == XUI_OK, "replace marker");
	iRet = xuiCodeAnnotationGetMarkers(pStore, 3, arrMarkers, 4, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 2, "marker count");
	XUI_TEST_CHECK(arrMarkers[0].iMarker == XUI_CODE_MARKER_BREAKPOINT && arrMarkers[0].iFlags == 9u, "marker replaced");
	XUI_TEST_CHECK(arrMarkers[0].sTooltip != NULL && strcmp(arrMarkers[0].sTooltip, "updated") == 0, "marker tooltip");
	iRet = xuiCodeAnnotationClearMarker(pStore, 3, XUI_CODE_MARKER_BOOKMARK);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear marker");
	iRet = xuiCodeAnnotationGetMarkers(pStore, 3, arrMarkers, 4, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1, "marker after clear");

	iRet = xuiCodeAnnotationSetIndicator(pStore, 1, XUI_CODE_INDICATOR_SQUIGGLE, 4, 9, 0u, 21u);
	XUI_TEST_CHECK(iRet == XUI_OK, "set indicator");
	iRet = xuiCodeAnnotationSetIndicator(pStore, 2, XUI_CODE_INDICATOR_BACKGROUND, 7, 12, 0u, 22u);
	XUI_TEST_CHECK(iRet == XUI_OK, "set second indicator");
	iRet = xuiCodeAnnotationGetIndicatorsAt(pStore, 8, arrIndicators, 4, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 2, "indicator hit count");
	XUI_TEST_CHECK(arrIndicators[0].iIndicator == 1 && arrIndicators[0].iUserData == 21u, "indicator data");
	iRet = xuiCodeAnnotationClearIndicator(pStore, 1, 5, 6);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear indicator overlap");
	iRet = xuiCodeAnnotationGetIndicatorsAt(pStore, 8, arrIndicators, 4, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 2 &&
		arrIndicators[0].iIndicator == 1 &&
		arrIndicators[0].tRange.iStart == 6 && arrIndicators[0].tRange.iEnd == 9,
		"indicator split preserves right side");
	iRet = xuiCodeAnnotationGetIndicatorsAt(pStore, 5, arrIndicators, 4, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 0, "indicator clear removes requested subrange");
	iRet = xuiCodeAnnotationGetIndicatorsAt(pStore, 4, arrIndicators, 4, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 &&
		arrIndicators[0].tRange.iStart == 4 && arrIndicators[0].tRange.iEnd == 5,
		"indicator split preserves left side");

	iRet = xuiCodeAnnotationStoreCreate(&pAuxStore);
	XUI_TEST_CHECK(iRet == XUI_OK && pAuxStore != NULL, "auxiliary store create");
	for ( i = 0; i < 16; i++ ) {
		iRet = xuiCodeAnnotationSetIndicator(pAuxStore, 10 + i,
			XUI_CODE_INDICATOR_UNDERLINE, i * 10, i * 10 + 8, 0u, (uintptr_t)i);
		XUI_TEST_CHECK(iRet == XUI_OK, "fill indicator capacity");
	}
	for ( iFail = 0; iFail < 2; iFail++ ) {
		g_iAnnotationFailAllocation = iFail;
		g_bAnnotationAllocationFailed = 0;
		iRet = xuiCodeAnnotationClearIndicator(pAuxStore, 10, 2, 4);
		g_iAnnotationFailAllocation = -1;
		XUI_TEST_CHECK(iRet == XUI_ERROR_OUT_OF_MEMORY && g_bAnnotationAllocationFailed,
			"indicator split allocation failure");
		iRet = xuiCodeAnnotationGetIndicatorsAt(pAuxStore, 3, arrIndicators, 4, &iCount);
		XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 &&
			arrIndicators[0].tRange.iStart == 0 && arrIndicators[0].tRange.iEnd == 8,
			"indicator split failure preserves store");
	}
	iRet = xuiCodeAnnotationClearIndicator(pAuxStore, 10, 2, 4);
	XUI_TEST_CHECK(iRet == XUI_OK, "indicator split succeeds after allocation failures");
	iRet = xuiCodeAnnotationGetIndicatorsAt(pAuxStore, 3, arrIndicators, 4, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 0, "indicator split cleared middle");
	iRet = xuiCodeAnnotationGetIndicatorsAt(pAuxStore, 1, arrIndicators, 4, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 && arrIndicators[0].tRange.iEnd == 2,
		"indicator split left range after allocation");
	iRet = xuiCodeAnnotationGetIndicatorsAt(pAuxStore, 5, arrIndicators, 4, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 && arrIndicators[0].tRange.iStart == 4,
		"indicator split right range after allocation");

	xuiCodeAnnotationStoreClear(pAuxStore);
	iRet = xuiCodeAnnotationSetIndicator(pAuxStore, 1, XUI_CODE_INDICATOR_UNDERLINE,
		10, 20, 0u, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK, "set tracked indicator");
	memset(&tDiagnostic, 0, sizeof(tDiagnostic));
	tDiagnostic.iSize = sizeof(tDiagnostic);
	tDiagnostic.tRange.iStart = 10;
	tDiagnostic.tRange.iEnd = 20;
	tDiagnostic.iSeverity = XUI_CODE_DIAGNOSTIC_WARNING;
	tDiagnostic.sMessage = "tracked";
	iRet = xuiCodeAnnotationSetDiagnostics(pAuxStore, &tDiagnostic, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "set tracked diagnostic");
	iDiagnosticVersion = xuiCodeAnnotationGetDiagnosticVersion(pAuxStore);
	iRet = xuiCodeAnnotationTrackEdit(pAuxStore, 15, 15, 18, 0, 0, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "track insertion inside annotation");
	iRet = xuiCodeAnnotationGetIndicatorsAt(pAuxStore, 22, arrIndicators, 4, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 &&
		arrIndicators[0].tRange.iStart == 10 && arrIndicators[0].tRange.iEnd == 23,
		"insertion preserves annotation suffix");
	iRet = xuiCodeAnnotationGetDiagnosticsAt(pAuxStore, 22, arrDiagnostics, 2, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 &&
		arrDiagnostics[0].tRange.iStart == 10 && arrDiagnostics[0].tRange.iEnd == 23 &&
		xuiCodeAnnotationGetDiagnosticVersion(pAuxStore) > iDiagnosticVersion,
		"insertion preserves diagnostic suffix and version");
	iDiagnosticVersion = xuiCodeAnnotationGetDiagnosticVersion(pAuxStore);
	iRet = xuiCodeAnnotationTrackEdit(pAuxStore, 12, 14, 16, 0, 0, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "track replacement inside annotation");
	iRet = xuiCodeAnnotationGetDiagnosticsAt(pAuxStore, 24, arrDiagnostics, 2, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 &&
		arrDiagnostics[0].tRange.iStart == 10 && arrDiagnostics[0].tRange.iEnd == 25,
		"replacement preserves diagnostic suffix");
	iDiagnosticVersion = xuiCodeAnnotationGetDiagnosticVersion(pAuxStore);
	iRet = xuiCodeAnnotationTrackEdit(pAuxStore, 12, 14, 14, 0, 0, 0);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiCodeAnnotationGetDiagnosticVersion(pAuxStore) > iDiagnosticVersion,
		"equal-length replacement touches diagnostic version");
	iDiagnosticVersion = xuiCodeAnnotationGetDiagnosticVersion(pAuxStore);
	iRet = xuiCodeAnnotationTrackEdit(pAuxStore, 30, 30, 33, 0, 0, 0);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiCodeAnnotationGetDiagnosticVersion(pAuxStore) == iDiagnosticVersion,
		"unrelated edit keeps diagnostic version");
	xuiCodeAnnotationStoreClear(pAuxStore);
	iRet = xuiCodeAnnotationSetIndicator(pAuxStore, 1, XUI_CODE_INDICATOR_UNDERLINE,
		INT_MAX - 2, INT_MAX, 0u, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK, "set overflow boundary indicator");
	iRet = xuiCodeAnnotationTrackEdit(pAuxStore, 0, 0, 10, 0, 0, 0);
	XUI_TEST_CHECK(iRet == XUI_ERROR_LIMIT_EXCEEDED, "annotation range overflow rejected");
	iRet = xuiCodeAnnotationGetIndicatorsAt(pAuxStore, INT_MAX - 1,
		arrIndicators, 4, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 &&
		arrIndicators[0].tRange.iStart == INT_MAX - 2 &&
		arrIndicators[0].tRange.iEnd == INT_MAX,
		"annotation range overflow preserves store");
	xuiCodeAnnotationStoreDestroy(pAuxStore);
	pAuxStore = NULL;

	memset(arrDiagnostics, 0, sizeof(arrDiagnostics));
	arrDiagnostics[0].iSize = sizeof(arrDiagnostics[0]);
	arrDiagnostics[0].tRange.iStart = 10;
	arrDiagnostics[0].tRange.iEnd = 14;
	arrDiagnostics[0].iSeverity = XUI_CODE_DIAGNOSTIC_ERROR;
	arrDiagnostics[0].sCode = "E001";
	arrDiagnostics[0].sMessage = "expected ;";
	arrDiagnostics[0].sSource = "c";
	arrDiagnostics[1].iSize = sizeof(arrDiagnostics[1]);
	arrDiagnostics[1].tRange.iStart = 20;
	arrDiagnostics[1].tRange.iEnd = 24;
	arrDiagnostics[1].iSeverity = XUI_CODE_DIAGNOSTIC_WARNING;
	arrDiagnostics[1].sCode = "W001";
	arrDiagnostics[1].sMessage = "unused";
	arrDiagnostics[1].sSource = "lint";
	iRet = xuiCodeAnnotationSetDiagnostics(pStore, arrDiagnostics, 2);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCodeAnnotationGetDiagnosticCount(pStore) == 2, "set diagnostics");
	iDiagnosticVersion = xuiCodeAnnotationGetDiagnosticVersion(pStore);
	XUI_TEST_CHECK(iDiagnosticVersion != 0u, "diagnostic version");
	memset(&tDiagnostic, 0, sizeof(tDiagnostic));
	iRet = xuiCodeAnnotationGetDiagnostic(pStore, 0, &tDiagnostic);
	XUI_TEST_CHECK(iRet == XUI_OK && tDiagnostic.iSeverity == XUI_CODE_DIAGNOSTIC_ERROR, "get diagnostic");
	XUI_TEST_CHECK(tDiagnostic.sMessage != arrDiagnostics[0].sMessage && strcmp(tDiagnostic.sMessage, "expected ;") == 0, "diagnostic string copied");
	iRet = xuiCodeAnnotationGetDiagnosticsInRange(pStore, 9, 21, arrDiagnostics, arrDiagnosticIndices, 2, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 2 && arrDiagnosticIndices[0] == 0 && arrDiagnosticIndices[1] == 1,
		"diagnostic indexed range query");
	iRet = xuiCodeAnnotationGetDiagnosticsAt(pStore, 11, arrDiagnostics, 2, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 && arrDiagnostics[0].iSeverity == XUI_CODE_DIAGNOSTIC_ERROR, "diagnostic hit");

	iRet = xuiCodeAnnotationTrackEdit(pStore, 6, 8, 11, 2, 2, 4);
	XUI_TEST_CHECK(iRet == XUI_OK, "track edit");
	iRet = xuiCodeAnnotationGetMarkers(pStore, 5, arrMarkers, 4, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 && arrMarkers[0].iMarker == XUI_CODE_MARKER_BREAKPOINT, "marker line shifted");
	iRet = xuiCodeAnnotationGetIndicatorsAt(pStore, 10, arrIndicators, 4, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 2 &&
		arrIndicators[0].iIndicator == 1 &&
		arrIndicators[0].tRange.iStart == 6 && arrIndicators[0].tRange.iEnd == 12 &&
		arrIndicators[1].iIndicator == 2 &&
		arrIndicators[1].tRange.iStart == 6 && arrIndicators[1].tRange.iEnd == 15,
		"indicator fragments shifted");
	iRet = xuiCodeAnnotationGetDiagnosticsAt(pStore, 14, arrDiagnostics, 2, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1, "diagnostic shifted");
	iRet = xuiCodeAnnotationTrackEdit(pStore, 9, 16, 9, 4, 5, 4);
	XUI_TEST_CHECK(iRet == XUI_OK, "track delete edit");
	iRet = xuiCodeAnnotationGetMarkers(pStore, 4, arrMarkers, 4, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1, "marker collapsed to delete start line");
	iRet = xuiCodeAnnotationGetIndicatorsAt(pStore, 6, arrIndicators, 4, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 2 &&
		arrIndicators[0].tRange.iStart == 6 && arrIndicators[0].tRange.iEnd == 9 &&
		arrIndicators[1].tRange.iStart == 6 && arrIndicators[1].tRange.iEnd == 9,
		"indicator fragments shrink after overlap delete");
	iRet = xuiCodeAnnotationGetDiagnosticsAt(pStore, 9, arrDiagnostics, 2, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 && arrDiagnostics[0].tRange.iStart == 9 && arrDiagnostics[0].tRange.iEnd == 10, "diagnostic collapsed after overlap delete");

	memset(arrReplacement, 0, sizeof(arrReplacement));
	arrReplacement[0].iSize = sizeof(arrReplacement[0]);
	arrReplacement[0].tRange.iStart = 30;
	arrReplacement[0].tRange.iEnd = 34;
	arrReplacement[0].sCode = "R1";
	arrReplacement[0].sMessage = "replacement one";
	arrReplacement[0].sSource = "test";
	arrReplacement[1].iSize = sizeof(arrReplacement[1]);
	arrReplacement[1].tRange.iStart = 40;
	arrReplacement[1].tRange.iEnd = 44;
	arrReplacement[1].sCode = "R2";
	arrReplacement[1].sMessage = "replacement two";
	arrReplacement[1].sSource = "test";
	bReplacementSucceeded = 0;
	for ( iFail = 0; iFail < 16; iFail++ ) {
		iDiagnosticVersion = xuiCodeAnnotationGetDiagnosticVersion(pStore);
		g_iAnnotationFailAllocation = iFail;
		g_bAnnotationAllocationFailed = 0;
		iRet = xuiCodeAnnotationSetDiagnostics(pStore, arrReplacement, 2);
		g_iAnnotationFailAllocation = -1;
		if ( iRet == XUI_OK ) {
			bReplacementSucceeded = 1;
			break;
		}
		XUI_TEST_CHECK(iRet == XUI_ERROR_OUT_OF_MEMORY && g_bAnnotationAllocationFailed,
			"diagnostic replacement allocation failure");
		XUI_TEST_CHECK(xuiCodeAnnotationGetDiagnosticCount(pStore) == 2 &&
			xuiCodeAnnotationGetDiagnosticVersion(pStore) == iDiagnosticVersion,
			"diagnostic replacement preserves old store");
		iRet = xuiCodeAnnotationGetDiagnostic(pStore, 0, &tDiagnostic);
		XUI_TEST_CHECK(iRet == XUI_OK && strcmp(tDiagnostic.sMessage, "expected ;") == 0,
			"diagnostic replacement preserves old strings");
	}
	XUI_TEST_CHECK(bReplacementSucceeded && xuiCodeAnnotationGetDiagnosticCount(pStore) == 2,
		"diagnostic replacement eventually succeeds");
	iRet = xuiCodeAnnotationGetDiagnostic(pStore, 0, &tDiagnostic);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(tDiagnostic.sMessage, "replacement one") == 0,
		"diagnostic replacement committed");

	iRet = xuiCodeAnnotationSetMarker(pStore, INT_MAX,
		XUI_CODE_MARKER_BOOKMARK, 0u, NULL, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK, "set maximum line marker");
	iRet = xuiCodeAnnotationTrackEdit(pStore, 0, 0, 1, 0, 0, 1);
	XUI_TEST_CHECK(iRet == XUI_ERROR_LIMIT_EXCEEDED, "track edit overflow rejected");
	iRet = xuiCodeAnnotationGetMarkers(pStore, INT_MAX, arrMarkers, 4, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1,
		"track edit overflow preserves marker");
	iRet = xuiCodeAnnotationGetDiagnosticsAt(pStore, INT_MAX,
		arrDiagnostics, 2, &iCount);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT,
		"maximum diagnostic offset rejected");
	iRet = xuiCodeAnnotationGetMarkers(pStore, 0, arrMarkers, -1, &iCount);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT,
		"negative marker capacity rejected");

	iRet = xuiCodeAnnotationClearDiagnostics(pStore);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCodeAnnotationGetDiagnosticCount(pStore) == 0, "clear diagnostics");
	XUI_TEST_CHECK(xuiCodeAnnotationGetDiagnosticVersion(pStore) > iDiagnosticVersion, "diagnostic version changes");
	iDiagnosticVersion = xuiCodeAnnotationGetDiagnosticVersion(pStore);
	iRet = xuiCodeAnnotationClearDiagnostics(pStore);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiCodeAnnotationGetDiagnosticVersion(pStore) == iDiagnosticVersion,
		"empty diagnostic clear keeps version");
	xuiCodeAnnotationStoreClear(pStore);
	iRet = xuiCodeAnnotationGetMarkers(pStore, 5, arrMarkers, 4, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 0, "clear store markers");

cleanup:
	xuiCodeAnnotationStoreDestroy(pAuxStore);
	xuiCodeAnnotationStoreDestroy(pStore);
	if ( iFailed ) return 1;
	printf("xui_code_annotation_test passed\n");
	return 0;
}
