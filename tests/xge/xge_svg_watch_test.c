/* Native content-generation observation. Uses the real SVG aggregate; no
 * renderer/window/device initialization, external resource or network. */
#define XRT_MODULE_ALL
#include "single/xge.c"
#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static const char svgText[]="<svg width='32' height='24'><g id='layer'><rect id='box' width='8' height='6'/></g></svg>";
typedef struct Watch {
    xge_svg_paint_watch_t link;
    xge_svg svg;
    struct Watch* peer;
    unsigned count;
    int mode;
} Watch;
static unsigned notifications;
static xge_svg create(void)
{
    xge_svg svg=NULL;assert(xgeSvgCreate(&svg)==XGE_OK);
    assert(xgeSvgLoadMemory(svg,svgText,(int)sizeof(svgText)-1)==XGE_OK);return svg;
}
static void notified(void* raw)
{
    Watch* watch=raw;assert(!watch->link.ppPrevious && !watch->link.pNext);
    ++watch->count;++notifications;
    if(watch->mode==1){
        /* Removal of another node must not leave a saved iterator dangling. */
        xgeSvgPaintLifetimeUnwatch(&watch->peer->link);xrtFree(watch->peer);
        watch->peer=NULL;xrtFree(watch);return;
    }
    if(watch->mode==2){
        xge_svg svg=watch->svg;watch->svg=NULL;
        assert(svg->iRefCount>=2);xgeSvgDestroy(svg);
        assert(svg->bClearing && svg->iRefCount>=1);
    }
    if(watch->mode==3){
        xge_svg clone=(xge_svg)(uintptr_t)1;
        assert(xgeSvgClear(watch->svg)==XGE_ERROR_BUSY);
        assert(xgeSvgLoadMemory(watch->svg,svgText,(int)sizeof(svgText)-1)==XGE_ERROR_BUSY);
        assert(xgeSvgClone(watch->svg,&clone)==XGE_ERROR_BUSY && !clone);
        xge_svg_paint_watch_t attempt={0};
        assert(xgeSvgPaintLifetimeWatch(&watch->svg->tPicturePaint,&attempt,notified,watch)==XGE_ERROR_BUSY);
        assert(!attempt.ppPrevious && !attempt.pNext);
    }
}
static void attach(Watch* watch,xge_svg svg)
{
    xge_svg_paint paint=NULL;assert(xgeSvgPaintGetByName(svg,"box",&paint)==XGE_OK);
    watch->svg=svg;assert(xgeSvgPaintLifetimeWatch(paint,&watch->link,notified,watch)==XGE_OK);
}
static int visit(xge_svg_paint paint,void* raw)
{
    Watch* watch=raw;
    if(!watch->link.ppPrevious)assert(xgeSvgPaintLifetimeWatch(paint,&watch->link,notified,watch)==XGE_OK);
    return 1;
}
int main(int argc,char** argv)
{
    assert(argc==2);int mode=atoi(argv[1]);assert(xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    xge_svg svg=create();Watch a={0},b={0},c={0};
    if(mode==0){
        xmemdebugsnapshot hotBefore,hotAfter;xrtMemDebugSnapshot(&hotBefore);
        attach(&a,svg);attach(&b,svg);attach(&c,svg);
        xgeSvgPaintLifetimeUnwatch(&b.link);xgeSvgPaintLifetimeUnwatch(&b.link);
        xrtMemDebugSnapshot(&hotAfter);
        assert(hotBefore.AllocCount==hotAfter.AllocCount && hotBefore.FreeCount==hotAfter.FreeCount && hotBefore.ReallocCount==hotAfter.ReallocCount);
        assert(xgeSvgClear(svg)==XGE_OK && a.count==1 && !b.count && c.count==1);
        assert(xgeSvgClear(svg)==XGE_OK && notifications==2);
    }else if(mode==1){
        Watch* peer=xrtCalloc(1,sizeof(*peer));Watch* self=xrtCalloc(1,sizeof(*self));assert(peer && self);
        attach(peer,svg);self->mode=1;self->peer=peer;attach(self,svg);
        assert(xgeSvgClear(svg)==XGE_OK && notifications==1 && !svg->pPaintWatches);
    }else if(mode==2){
        a.mode=2;attach(&a,svg);assert(xgeSvgClear(svg)==XGE_OK);
        assert(a.count==1 && !a.svg);svg=NULL; /* The callback dropped the external owner. */
    }else if(mode==3 || mode==4){
        a.mode=3;attach(&a,svg);
        if(mode==3)assert(xgeSvgClear(svg)==XGE_OK);
        else assert(xgeSvgLoadMemory(svg,svgText,(int)sizeof(svgText)-1)==XGE_OK);
        assert(a.count==1 && !a.link.ppPrevious);
        if(mode==4){
            a.mode=0;attach(&a,svg);
            /* Argument rejection preserves the old generation; once replacement
             * starts, even allocation failure invalidates its borrowed paints. */
            assert(xgeSvgLoadMemory(svg,svgText,0)==XGE_ERROR_INVALID_ARGUMENT);
            assert(a.count==1 && a.link.ppPrevious);
            assert(xrtMemDebugFailAfter(0));
            assert(xgeSvgLoadMemory(svg,svgText,(int)sizeof(svgText)-1)==XGE_ERROR_OUT_OF_MEMORY);
            assert(xrtMemDebugFailTriggered());xrtMemDebugFailClear();
            assert(a.count==2 && !a.link.ppPrevious);
            assert(xgeSvgLoadMemory(svg,svgText,(int)sizeof(svgText)-1)==XGE_OK);
            attach(&a,svg);assert(xgeSvgClear(svg)==XGE_OK && a.count==3);
        }
    }else if(mode==5){
        xge_svg_paint picture=NULL;assert(xgeSvgPaintGetPicture(svg,&picture)==XGE_OK);
        a.svg=svg;assert(xgeSvgPaintTraverse(picture,visit,&a)==XGE_OK);
        assert(a.link.ppPrevious && !a.count);assert(xgeSvgClear(svg)==XGE_OK && a.count==1);
    }else if(mode==6){
        attach(&a,svg);xge_svg clone=NULL;assert(xgeSvgClone(svg,&clone)==XGE_OK);
        attach(&b,clone);assert(xgeSvgClear(clone)==XGE_OK && b.count==1 && !a.count);
        xgeSvgDestroy(clone);assert(xgeSvgClear(svg)==XGE_OK && a.count==1);
    }else if(mode==7){
        xge_svg_paint paint=NULL;assert(xgeSvgPaintGetPicture(svg,&paint)==XGE_OK);
        assert(xgeSvgPaintLifetimeWatch(NULL,&a.link,notified,&a)==XGE_ERROR_INVALID_ARGUMENT);
        assert(xgeSvgPaintLifetimeWatch(paint,NULL,notified,&a)==XGE_ERROR_INVALID_ARGUMENT);
        assert(xgeSvgPaintLifetimeWatch(paint,&a.link,NULL,&a)==XGE_ERROR_INVALID_ARGUMENT);
        attach(&a,svg);assert(xgeSvgPaintLifetimeWatch(paint,&a.link,notified,&b)==XGE_ERROR_INVALID_STATE);
        assert(xrtMemDebugFailAfter(0));attach(&b,svg);xgeSvgPaintLifetimeUnwatch(&b.link);
        assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();
        xgeSvgDestroy(svg);svg=NULL;assert(a.count==1 && !b.count);
    }else assert(!"unknown mode");
    xgeSvgPaintLifetimeUnwatch(&a.link);xgeSvgPaintLifetimeUnwatch(&b.link);xgeSvgPaintLifetimeUnwatch(&c.link);
    xgeSvgPaintLifetimeUnwatch(NULL);xgeSvgDestroy(svg);
    xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    assert(before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
    printf("SVG watches mode %d: %u notifications, %zu-byte document, exact heap baseline\n",mode,notifications,sizeof(struct xge_svg_t));
    return 0;
}
