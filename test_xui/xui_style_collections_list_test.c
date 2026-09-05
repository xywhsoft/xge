#include "../src/xui_list_view.c"
#include "xui_style_collections_test.h"
#define K(key, field) {"listview." key, offsetof(xui_list_view_data_t, field)}
static const collection_key_t keys[]={
    K("background.color",iBackgroundColor), K("border.color",iBorderColor), K("focus.color",iFocusColor),
    K("row.color",iRowColor), K("row.hover_color",iHoverColor), K("row.selected_color",iSelectedColor),
    K("text.color",iTextColor), K("text.disabled_color",iDisabledTextColor), K("text.selected_color",iSelectedTextColor),
    K("scrollbar.track_color",iTrackColor), K("scrollbar.thumb_color",iThumbColor),
    K("scrollbar.hover_color",iScrollbarHoverColor), K("scrollbar.active_color",iScrollbarActiveColor),
    K("scrollbar.focus_color",iScrollbarFocusColor), K("scrollbar.disabled_color",iScrollbarDisabledColor)
};
static void resolve(xui_widget w,void* b,void* e) { __xuiListViewResolve(w,b,e); }
int main(void)
{
    xui_widget w; xui_list_view_data_t* d; xui_list_view_desc_t desc={0}; const char* items[50]; int i;
    for(i=0;i<COUNT(items);i++) items[i]="list row";
    collectionInit(); desc.iSize=sizeof(desc); desc.arrItems=items; desc.iItemCount=COUNT(items); desc.iSelected=0;
    CHECK(xuiListViewCreate(gContext,&w,&desc)==XUI_OK); d=__xuiListViewGetData(w);
    collectionAttach(w);
    collectionVerifyKeys(w,d,sizeof(*d),resolve,keys,COUNT(keys));
    collectionVerifyRenderedCascade(w,"listview.text.selected_color",d->iSelectedTextColor);
    collectionInline(w,"listview.text.selected_color",COLOR_A); collectionRender(); CHECK(collectionSeen(COLOR_A));
    collectionInline(w,"listview.text.selected_color",COLOR_B); collectionRender(); CHECK(collectionSeen(COLOR_B)); CHECK(!collectionSeen(COLOR_A));
    collectionInline(w,"listview.text.selected_color",0); collectionRender(); CHECK(!collectionSeen(COLOR_B));
    CHECK(xuiWidgetSetInlineStyle(w,NULL,0)==XUI_OK); collectionRender(); CHECK(collectionSeen(XUI_COLOR_WHITE));
    collectionInline(w,"listview.scrollbar.thumb_color",COLOR_C); collectionRender(); CHECK(collectionSeen(COLOR_C));
    CHECK(xuiListViewSetScrollbarColors(w,COLOR_A,COLOR_D,COLOR_A,COLOR_A,COLOR_A,COLOR_A)==XUI_OK);
    collectionRender(); CHECK(collectionSeen(COLOR_C)); CHECK(!collectionSeen(COLOR_D));
    CHECK(xuiWidgetSetInlineStyle(w,NULL,0)==XUI_OK); collectionRender(); CHECK(collectionSeen(COLOR_D));
    collectionInline(w,"listview.scrollbar.thumb_color",0); collectionRender(); CHECK(!collectionSeen(COLOR_D));
    collectionDone(); puts("style_collections list: PASS"); return 0;
}
