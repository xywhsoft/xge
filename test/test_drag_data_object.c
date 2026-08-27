#include <stdio.h>
#include <string.h>

#include "../xge.c"

#define TEST_CHECK(expr, message) \
	do { if ( !(expr) ) { printf("test_drag_data_object failed: %s\n", message); return 1; } } while ( 0 )

static int g_iProviderFreed;
static int g_iCancelCount;

static int __testProvider(const char* sFormat, void* pOutput, size_t iCapacity,
	size_t* pOutputSize, void* pUser)
{
	const char* sText = (const char*)pUser;
	size_t iSize = strlen(sText);
	(void)sFormat;
	*pOutputSize = iSize;
	if ( pOutput == NULL ) return XGE_OK;
	if ( iCapacity < iSize ) return XGE_ERROR_BUFFER_TOO_SMALL;
	memcpy(pOutput, sText, iSize);
	return XGE_OK;
}

static void __testProviderFree(void* pUser)
{
	(void)pUser;
	g_iProviderFreed++;
}

static uint32_t __testDragEvent(const xge_drag_event_t* pEvent, void* pUser)
{
	(void)pUser;
	if ( pEvent->iType == XGE_DRAG_EVENT_CANCEL ) g_iCancelCount++;
	return XGE_DRAG_EFFECT_NONE;
}

int main(void)
{
	xge_data_object pData;
	xge_drag_drop_caps_t tCaps;
	char sBuffer[32];
	size_t iSize;

	TEST_CHECK(xgeDataObjectCreate(&pData) == XGE_OK, "create");
	TEST_CHECK(xgeDataObjectSet(pData, XGE_DATA_FORMAT_TEXT_UTF8, "hello", 5u) == XGE_OK, "set text");
	TEST_CHECK(xgeDataObjectSetProvider(pData, "application/test", __testProvider,
		__testProviderFree, (void*)"lazy") == XGE_OK, "set provider");
	TEST_CHECK(xgeDataObjectFormatCount(pData) == 2, "format count");
	TEST_CHECK(xgeDataObjectHas(pData, XGE_DATA_FORMAT_TEXT_UTF8), "has text");
	iSize = 0u;
	TEST_CHECK(xgeDataObjectGet(pData, XGE_DATA_FORMAT_TEXT_UTF8, NULL, 0u, &iSize) == XGE_OK && iSize == 5u, "measure text");
	memset(sBuffer, 0, sizeof(sBuffer));
	TEST_CHECK(xgeDataObjectGet(pData, "application/test", sBuffer, sizeof(sBuffer), &iSize) == XGE_OK && iSize == 4u && memcmp(sBuffer, "lazy", 4u) == 0, "read provider");
	memset(&tCaps, 0, sizeof(tCaps));
	tCaps.iSize = sizeof(tCaps);
	TEST_CHECK(xgeDragDropCapsGet(&tCaps) == XGE_OK && tCaps.bInternalOnly, "caps");
	TEST_CHECK(xgeDragEventCallbackSet(__testDragEvent, NULL) == XGE_OK, "event callback");
	TEST_CHECK(xgeDragBegin(pData, XGE_DRAG_EFFECT_COPY | XGE_DRAG_EFFECT_MOVE,
		XGE_DRAG_EFFECT_COPY) == XGE_OK, "begin");
	TEST_CHECK(xgeDragIsActive(), "active");
	TEST_CHECK(xgeDragCancel() == XGE_OK && !xgeDragIsActive() && g_iCancelCount == 1, "cancel");
	xgeDataObjectRelease(pData);
	TEST_CHECK(g_iProviderFreed == 1, "provider cleanup");
	printf("test_drag_data_object passed\n");
	return 0;
}
