#include "xge.h"

static xge_resource_t resource;

int testEngineInit(void)
{
	if ( !xrtMemDebugEnable(true) ) return XGE_ERROR_INVALID_STATE;
	xge_desc_t desc = {0};
	desc.iWidth = 64;
	desc.iHeight = 32;
	desc.iTargetFPS = 60;
	desc.iRunMode = XGE_RUN_MANUAL;
	return xgeInit(&desc);
}

int testMemoryFail(int budget)
{
	if ( budget >= 0 ) return xrtMemDebugFailAfter(budget);
	xrtMemDebugFailClear();
	xrtClearError();
	return 1;
}

int testResourceLoad(const char* uri)
{
	xgeResourceFree(&resource);
	return xgeResourceLoad(uri, &resource);
}

int testResourceSize(void)
{
	return resource.iSize;
}

int testResourceByte(int index)
{
	return index < 0 || index >= resource.iSize ? -1 : ((const unsigned char*)resource.pData)[index];
}

int testEngineUnit(void)
{
	xgeResourceFree(&resource);
	return xgeUnitChecked();
}
