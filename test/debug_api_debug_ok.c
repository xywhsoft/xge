#include "../xge.h"

int main(void)
{
	xge_debug_stats_t Stats;
	char szCaps[64];

	(void)xgeDebugGetStats(&Stats);
	(void)xgeDebugDumpCaps(szCaps, sizeof(szCaps));
	(void)xgedbgDirtyRectCount();
	(void)xgedbgXuiWidgetTreeDump(NULL, szCaps, sizeof(szCaps));
	(void)xgedbgXuiLayoutSnapshot(NULL, szCaps, sizeof(szCaps));
	(void)xgedbgXuiWidgetInspect(NULL, NULL, NULL);
	(void)xgedbgXuiWidgetInspectAt(NULL, 0.0f, 0.0f, NULL);
	(void)xgedbgXuiDebugOverlayPaint(NULL, 0, NULL);
	(void)xgedbgXuiPageTrace(NULL, szCaps, sizeof(szCaps));
	return 0;
}
