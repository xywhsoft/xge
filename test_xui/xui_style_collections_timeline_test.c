#include "../src/xui_timeline_view.c"
#include "xui_style_collections_test.h"
#define K(key, field) {"timelineview." key, offsetof(xui_timeline_paint_t,tColors)+offsetof(xui_timeline_view_colors_t,field)}
static const collection_key_t keys[]={
    K("background.color",iBackgroundColor),
    K("corner.color",iCornerColor),
    K("ruler.color",iRulerColor),
    K("layer.color",iLayerColor),
    K("layer.alt_color",iLayerAltColor),
    K("grid.color",iGridColor),
    K("grid.strong_color",iGridStrongColor),
    K("text.color",iTextColor),
    K("text.muted_color",iMutedTextColor),
    K("selection.color",iSelectedColor),
    K("hover.color",iHoverColor),
    K("current.color",iCurrentColor),
    K("frame.color",iFrameColor),
    K("frame.key_color",iKeyFrameColor),
    K("frame.blank_key_color",iBlankKeyFrameColor),
    K("span.color",iSpanColor),
    K("span.text_color",iSpanTextColor),
    K("icon.color",iIconColor),
    K("icon.hidden_color",iHiddenIconColor),
    K("icon.locked_color",iLockedIconColor),
    K("border.color",iBorderColor),
    K("focus.color",iFocusColor),
    K("disabled.color",iDisabledColor),
    K("scrollbar.track_color",iTrackColor),
    K("scrollbar.thumb_color",iThumbColor),
    K("scrollbar.hover_color",iScrollbarHoverColor),
    K("scrollbar.active_color",iScrollbarActiveColor),
    K("scrollbar.focus_color",iScrollbarFocusColor),
    K("scrollbar.disabled_color",iScrollbarDisabledColor),
    {"timelineview.layer.accent_color",offsetof(xui_timeline_paint_t,iLayerAccentColor)}
};
static void resolve(xui_widget w,void* b,void* e) { __xuiTimeLineResolvePaint(w,__xuiTimeLineViewGetData(w),e); }
int main(void)
{
    xui_widget w; xui_timeline_view_data_t* d; xui_timeline_paint_t basePaint;
    xui_timeline_view_colors_t baseColors, readColors; xui_timeline_span_t span;
    int i, layer, defaultSpan, customSpan, removedSpan, shiftedSpan, croppedSpan;
    int layerChanges, frameChanges, spanChanges;
    collectionInit(); CHECK(xuiTimeLineViewCreate(gContext,&w,NULL)==XUI_OK);
    CHECK(xuiTimeLineViewSetFrameCount(w,100)==XUI_OK);
    for(i=0;i<30;i++) CHECK(xuiTimeLineViewAddLayer(w,"layer",&layer)==XUI_OK);
    CHECK(xuiTimeLineViewSetLayerColor(w,2,0xe37b21ffu)==XUI_OK);
    CHECK(xuiTimeLineViewSetLayerColor(w,5,0)==XUI_OK);
    CHECK(xuiTimeLineViewSetLayerVisible(w,3,0)==XUI_OK);
    CHECK(xuiTimeLineViewSetLayerLocked(w,4,1)==XUI_OK);
    CHECK(xuiTimeLineViewSetFrame(w,0,1,XUI_TIMELINE_FRAME_NORMAL,NULL)==XUI_OK);
    CHECK(xuiTimeLineViewSetFrame(w,0,2,XUI_TIMELINE_FRAME_KEY,NULL)==XUI_OK);
    CHECK(xuiTimeLineViewSetFrame(w,0,3,XUI_TIMELINE_FRAME_BLANK_KEY,NULL)==XUI_OK);
    CHECK(xuiTimeLineViewAddSpan(w,1,1,4,XUI_TIMELINE_SPAN_MOTION,"default",&defaultSpan)==XUI_OK);
    CHECK(xuiTimeLineViewAddSpan(w,2,1,4,XUI_TIMELINE_SPAN_MOTION,"custom",&customSpan)==XUI_OK);
    CHECK(xuiTimeLineViewSetSpanColor(w,customSpan,0x249173ffu)==XUI_OK);
    CHECK(xuiTimeLineViewSelectFrame(w,0,7,1)==XUI_OK);
    d=__xuiTimeLineViewGetData(w); collectionAttach(w);
    d->iHoverLayer=1; CHECK(xuiSetFocusWidget(gContext,w)==XUI_OK);
    CHECK(xuiWidgetInvalidate(w,XUI_WIDGET_DIRTY_CACHE|XUI_WIDGET_DIRTY_RENDER)==XUI_OK);
    collectionRender();
    __xuiTimeLineResolvePaint(w,d,&basePaint); baseColors=d->tColors;
    layerChanges=d->iLayerChangeCount; frameChanges=d->iFrameChangeCount; spanChanges=d->iSpanChangeCount;
    collectionVerifyKeys(w,&basePaint,sizeof(basePaint),resolve,keys,COUNT(keys));
    for(i=0;i<COUNT(keys);i++) {
        if(!strcmp(keys[i].sKey,"timelineview.disabled.color")) CHECK(xuiWidgetSetEnabled(w,0)==XUI_OK);
        collectionInline(w,keys[i].sKey,COLOR_C); collectionRender();
        if(!strstr(keys[i].sKey,"scrollbar.") || strstr(keys[i].sKey,"track_color") || strstr(keys[i].sKey,"thumb_color")) {
            if(!collectionSeen(COLOR_C)) fprintf(stderr,"not rendered: %s\n",keys[i].sKey);
            CHECK(collectionSeen(COLOR_C));
        }
        collectionInline(w,keys[i].sKey,0); collectionRender(); CHECK(!collectionSeen(COLOR_C));
        if(!strcmp(keys[i].sKey,"timelineview.disabled.color")) CHECK(xuiWidgetSetEnabled(w,1)==XUI_OK);
    }
    collectionInline(w,"timelineview.span.color",COLOR_C); collectionRender();
    CHECK(collectionSeen(COLOR_C)); CHECK(collectionSeen(0x249173ffu)); CHECK(collectionSeen(0xe37b21ffu));
    baseColors.iSpanColor=COLOR_D; CHECK(xuiTimeLineViewSetColors(w,&baseColors)==XUI_OK);
    collectionRender(); CHECK(collectionSeen(COLOR_C)); CHECK(!collectionSeen(COLOR_D));
    CHECK(xuiTimeLineViewGetColors(w,&readColors)==XUI_OK); CHECK(readColors.iSpanColor==COLOR_D);
    CHECK(xuiWidgetSetInlineStyle(w,NULL,0)==XUI_OK); collectionRender();
    CHECK(collectionSeen(COLOR_D)); CHECK(collectionSeen(0x249173ffu));
    CHECK(d->iLayerChangeCount==layerChanges); CHECK(d->iFrameChangeCount==frameChanges); CHECK(d->iSpanChangeCount==spanChanges);
    /* Side flags must follow the records through compaction and layer reordering. */
    CHECK(xuiTimeLineViewMoveLayer(w,2,6)==XUI_OK); CHECK(d->arrLayerCustomColor[6]==1);
    CHECK(xuiTimeLineViewMoveLayer(w,6,2)==XUI_OK); CHECK(d->arrLayerCustomColor[2]==1);
    CHECK(xuiTimeLineViewRemoveLayer(w,0)==XUI_OK);
    CHECK(d->arrLayerCustomColor[1]==1 && d->arrLayers[1].iColor==0xe37b21ffu);
    CHECK(d->arrLayerCustomColor[4]==1 && d->arrLayers[4].iColor==0);
    CHECK(xuiTimeLineViewAddSpan(w,0,8,9,XUI_TIMELINE_SPAN_MOTION,"remove",&removedSpan)==XUI_OK);
    CHECK(xuiTimeLineViewAddSpan(w,0,10,11,XUI_TIMELINE_SPAN_MOTION,"keep",&shiftedSpan)==XUI_OK);
    CHECK(xuiTimeLineViewSetSpanColor(w,shiftedSpan,COLOR_B)==XUI_OK);
    CHECK(xuiTimeLineViewRemoveSpan(w,removedSpan)==XUI_OK);
    i=__xuiTimeLineFindSpan(d,shiftedSpan); CHECK(i>=0 && d->arrSpanCustomColor[i]);
    CHECK(xuiTimeLineViewAddSpan(w,0,80,90,XUI_TIMELINE_SPAN_MOTION,"crop",&croppedSpan)==XUI_OK);
    CHECK(xuiTimeLineViewAddSpan(w,0,12,13,XUI_TIMELINE_SPAN_MOTION,"shift",&shiftedSpan)==XUI_OK);
    CHECK(xuiTimeLineViewSetSpanColor(w,shiftedSpan,COLOR_B)==XUI_OK);
    CHECK(xuiTimeLineViewSetFrameCount(w,40)==XUI_OK);
    CHECK(__xuiTimeLineFindSpan(d,croppedSpan)<0);
    i=__xuiTimeLineFindSpan(d,shiftedSpan); CHECK(i>=0 && d->arrSpanCustomColor[i]);
    CHECK(xuiTimeLineViewGetSpan(w,customSpan,&span)==XUI_OK); CHECK(span.iColor==0x249173ffu);
    collectionDone(); puts("style_collections timeline: PASS"); return 0;
}
