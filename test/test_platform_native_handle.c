#include <stdio.h>
#include <string.h>

#include "../xge.c"

#define TEST_CHECK(expr, message) \
	do { \
		if ( !(expr) ) { \
			printf("test_platform_native_handle failed: %s\n", (message)); \
			return 1; \
		} \
	} while ( 0 )

int main(void)
{
	xge_platform_backend_t tBackend;

	TEST_CHECK(xgePlatformNativeHandle() == NULL, "handle before initialization");
	memset(&g_xge, 0, sizeof(g_xge));
	g_xge.bInitialized = 1;
	g_xge.bSokolRunning = 1;
	memset(&tBackend, 0, sizeof(tBackend));
	tBackend.iType = XGE_PLATFORM_BACKEND_CUSTOM;
	tBackend.sName = "test-custom";
	g_xge.tPlatformBackend = tBackend;
	TEST_CHECK(xgePlatformNativeHandle() == NULL, "custom backend has no native handle");
	g_xge.bSokolRunning = 0;
	TEST_CHECK(xgePlatformNativeHandle() == NULL, "handle after platform shutdown");
	memset(&g_xge, 0, sizeof(g_xge));
	printf("test_platform_native_handle passed\n");
	return 0;
}
