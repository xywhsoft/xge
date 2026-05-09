#include "../xge.h"

int main(void)
{
	char szCaps[64];
	return xgeDebugDumpCaps(szCaps, sizeof(szCaps)) + xgedbgDirtyRectCount() + xgedbgXuiLayoutSnapshot(NULL, szCaps, sizeof(szCaps)) + xgedbgXuiPageTrace(NULL, szCaps, sizeof(szCaps)) + xgedbgXuiEventTrace(NULL, NULL, szCaps, sizeof(szCaps));
}
