#include "xui.h"

#include <stdio.h>
#include <string.h>

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
	xui_code_marker_t arrMarkers[4];
	xui_code_indicator_t arrIndicators[4];
	xui_code_diagnostic_t arrDiagnostics[2];
	xui_code_diagnostic_t tDiagnostic;
	int iCount;
	int iFailed;
	int iRet;

	pStore = NULL;
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
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 && arrIndicators[0].iIndicator == 2, "indicator after clear");

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
	memset(&tDiagnostic, 0, sizeof(tDiagnostic));
	iRet = xuiCodeAnnotationGetDiagnostic(pStore, 0, &tDiagnostic);
	XUI_TEST_CHECK(iRet == XUI_OK && tDiagnostic.iSeverity == XUI_CODE_DIAGNOSTIC_ERROR, "get diagnostic");
	XUI_TEST_CHECK(tDiagnostic.sMessage != arrDiagnostics[0].sMessage && strcmp(tDiagnostic.sMessage, "expected ;") == 0, "diagnostic string copied");
	iRet = xuiCodeAnnotationGetDiagnosticsAt(pStore, 11, arrDiagnostics, 2, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 && arrDiagnostics[0].iSeverity == XUI_CODE_DIAGNOSTIC_ERROR, "diagnostic hit");

	iRet = xuiCodeAnnotationTrackEdit(pStore, 6, 8, 11, 2, 2, 4);
	XUI_TEST_CHECK(iRet == XUI_OK, "track edit");
	iRet = xuiCodeAnnotationGetMarkers(pStore, 5, arrMarkers, 4, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 && arrMarkers[0].iMarker == XUI_CODE_MARKER_BREAKPOINT, "marker line shifted");
	iRet = xuiCodeAnnotationGetIndicatorsAt(pStore, 10, arrIndicators, 4, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 && arrIndicators[0].iIndicator == 2, "indicator shifted");
	iRet = xuiCodeAnnotationGetDiagnosticsAt(pStore, 14, arrDiagnostics, 2, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1, "diagnostic shifted");
	iRet = xuiCodeAnnotationTrackEdit(pStore, 9, 16, 9, 4, 5, 4);
	XUI_TEST_CHECK(iRet == XUI_OK, "track delete edit");
	iRet = xuiCodeAnnotationGetMarkers(pStore, 4, arrMarkers, 4, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1, "marker collapsed to delete start line");
	iRet = xuiCodeAnnotationGetIndicatorsAt(pStore, 6, arrIndicators, 4, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 && arrIndicators[0].tRange.iStart == 6 && arrIndicators[0].tRange.iEnd == 9, "indicator shrinks after overlap delete");
	iRet = xuiCodeAnnotationGetDiagnosticsAt(pStore, 9, arrDiagnostics, 2, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 && arrDiagnostics[0].tRange.iStart == 9 && arrDiagnostics[0].tRange.iEnd == 10, "diagnostic collapsed after overlap delete");

	iRet = xuiCodeAnnotationClearDiagnostics(pStore);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCodeAnnotationGetDiagnosticCount(pStore) == 0, "clear diagnostics");
	xuiCodeAnnotationStoreClear(pStore);
	iRet = xuiCodeAnnotationGetMarkers(pStore, 5, arrMarkers, 4, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 0, "clear store markers");

cleanup:
	xuiCodeAnnotationStoreDestroy(pStore);
	if ( iFailed ) return 1;
	printf("xui_code_annotation_test passed\n");
	return 0;
}
