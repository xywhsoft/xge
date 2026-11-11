/* Old-code assertion control stops before a reentrant double free. */
#define XRT_MODULE_ALL
#include "single/xge.c"
#undef NDEBUG
#include <assert.h>
#include <stdio.h>
static const char text[]="<svg width='4' height='3'><rect width='4' height='3'/></svg>";
static int supply(const xstrview* path,xge_resource_output_t* output,void* raw)
{assert(path && path->Data);(void)raw;return xgeResourceOutputSet(output,text,sizeof(text)-1);}
static void invalidated(void* raw)
{
    ++*(unsigned*)raw;
    assert(g_xgeSvgCacheHead==NULL); /* Old code stops here before unsafe reentry. */
    xgeSvgCacheClear();xge_svg added=NULL;
    assert(xgeSvgLoadCached("cache-reset://new",&added)==XGE_OK);xgeSvgDestroy(added);
}
int main(void)
{
    assert(xrtMemDebugEnable(true));xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    assert(xgeResourceProviderAddOwned("cache-reset",supply,NULL,NULL)==XGE_OK);
    xge_svg svg=NULL;assert(xgeSvgLoadCached("cache-reset://old",&svg)==XGE_OK);
    xge_svg_paint paint=NULL;assert(xgeSvgPaintGetPicture(svg,&paint)==XGE_OK);
    xge_svg_paint_watch_t link={0};unsigned notifications=0;
    assert(xgeSvgPaintLifetimeWatch(paint,&link,invalidated,&notifications)==XGE_OK);
    xgeSvgDestroy(svg);xgeSvgCacheClear();
    assert(notifications==1 && !link.ppPrevious && g_xgeSvgCacheHead);
    assert(xgeSvgCacheInvalidate("cache-reset://new")==XGE_OK && !g_xgeSvgCacheHead);
    assert(xgeResourceProviderClearChecked()==XGE_OK);xrtClearError();xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    assert(before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
    puts("Legacy cache reset detaches before notification and preserves reentrant publication");return 0;
}
