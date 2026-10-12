/* ch86 — 度量与缓存统计 */
#include "tut_capture.h"

#define FONT_PATH "C:/Windows/Fonts/segoeui.ttf"

static void draw_scene(void)
{
	xge_font_t font;
	xge_font_metrics_t metrics;
	xge_font_cache_stats_t stats;
	char buf[128];
	int ret;

	ret = xgeFontLoad(&font, FONT_PATH, 28.0f);
	if (ret != XGE_OK) return;

	/* 绘制文本以填充缓存 */
	xgeTextDraw(&font, "Font Metrics Demo", 60.0f, 80.0f, XGE_COLOR_RGBA(255, 255, 255, 255));

	/* 获取度量信息 */
	memset(&metrics, 0, sizeof(metrics));
	metrics.iSize = sizeof(metrics);
	xgeFontGetMetrics(&font, &metrics);

	snprintf(buf, sizeof(buf), "Ascent: %.1f  Descent: %.1f", metrics.fAscent, metrics.fDescent);
	xgeTextDraw(&font, buf, 60.0f, 140.0f, XGE_COLOR_RGBA(79, 216, 194, 255));

	snprintf(buf, sizeof(buf), "LineHeight: %.1f  LineGap: %.1f", metrics.fLineHeight, metrics.fLineGap);
	xgeTextDraw(&font, buf, 60.0f, 185.0f, XGE_COLOR_RGBA(79, 216, 194, 255));

	/* 缓存统计 */
	memset(&stats, 0, sizeof(stats));
	stats.iSize = sizeof(stats);
	xgeFontCacheGetStats(&font, &stats);

	snprintf(buf, sizeof(buf), "Glyphs: %u  AtlasPages: %u", stats.iGlyphCount, stats.iAtlasPageCount);
	xgeTextDraw(&font, buf, 60.0f, 240.0f, XGE_COLOR_RGBA(255, 200, 80, 255));

	xgeFontFree(&font);
}

int main(int argc, char** argv) { return tut_run(draw_scene, "ch86", argc, argv); }
