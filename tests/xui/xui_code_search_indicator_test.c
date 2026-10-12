#include "xui.h"

#include <stdio.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_code_search_indicator_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

int main(void)
{
	xui_code_document pDocument;
	xui_code_annotation_store pStore;
	xui_code_indicator_t arrIndicators[8];
	char sError[128];
	int iCount;
	int iFailed;
	int iRet;

	pDocument = NULL;
	pStore = NULL;
	iFailed = 0;

	iRet = xuiCodeDocumentCreate(&pDocument);
	XUI_TEST_CHECK(iRet == XUI_OK, "document create");
	iRet = xuiCodeAnnotationStoreCreate(&pStore);
	XUI_TEST_CHECK(iRet == XUI_OK, "store create");
	iRet = xuiCodeDocumentSetText(pDocument, "foo Foo food\nfoo_1 foo_2\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "document text");

	iRet = xuiCodeSearchMarkAllPlain(pDocument, pStore, "foo", XUI_CODE_SEARCH_WHOLE_WORD, 0, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 2, "plain mark all count");
	iRet = xuiCodeAnnotationGetIndicatorsAt(pStore, 1, arrIndicators, 8, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 && arrIndicators[0].iIndicator == XUI_CODE_INDICATOR_SEARCH_RESULT, "plain indicator first");
	iRet = xuiCodeAnnotationGetIndicatorsAt(pStore, 8, arrIndicators, 8, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 0, "whole word skipped food");
	iRet = xuiCodeAnnotationGetIndicatorsAt(pStore, 5, arrIndicators, 8, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1, "plain indicator second");

	iRet = xuiCodeSearchMarkAllRegex(pDocument, pStore, "foo_[0-9]", XUI_CODE_SEARCH_CASE_SENSITIVE, 123, &iCount, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 2, "regex mark all count");
	iRet = xuiCodeAnnotationGetIndicatorsAt(pStore, 15, arrIndicators, 8, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 && arrIndicators[0].iIndicator == 123, "regex custom indicator");
	iRet = xuiCodeAnnotationGetIndicatorsAt(pStore, 1, arrIndicators, 8, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 && arrIndicators[0].iIndicator == XUI_CODE_INDICATOR_SEARCH_RESULT, "other indicator preserved");
	iRet = xuiCodeSearchClearResultIndicators(pStore, 123);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear regex indicators");
	iRet = xuiCodeAnnotationGetIndicatorsAt(pStore, 15, arrIndicators, 8, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 0, "regex indicators cleared");
	iRet = xuiCodeSearchClearResultIndicators(pStore, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear default indicators");
	iRet = xuiCodeAnnotationGetIndicatorsAt(pStore, 1, arrIndicators, 8, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 0, "default indicators cleared");

cleanup:
	xuiCodeAnnotationStoreDestroy(pStore);
	xuiCodeDocumentDestroy(pDocument);
	if ( iFailed ) return 1;
	printf("xui_code_search_indicator_test passed\n");
	return 0;
}
