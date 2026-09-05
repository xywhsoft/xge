#include "../src/xui_internal.h"
static int gTreeWork;
#define XUI_TREE_VIEW_TEST_COUNT(field,count) (gTreeWork+=(count))
#include "../src/xui_tree_view.c"
#include "xui_style_collections_test.h"
#define K(key, field) {"treeview." key, offsetof(xui_tree_view_data_t, field)}
static const collection_key_t keys[]={
    K("background.color",iBackgroundColor),
    K("border.color",iBorderColor),
    K("focus.color",iFocusColor),
    K("row.color",iRowColor),
    K("row.hover_color",iHoverColor),
    K("row.selected_color",iSelectedColor),
    K("text.color",iTextColor),
    K("text.disabled_color",iDisabledTextColor),
    K("expander.color",iExpanderColor),
    K("icon.color",iIconColor),
    K("check.color",iCheckColor),
    K("text.selected_color",iSelectedTextColor),
    K("expander.selected_color",iSelectedExpanderColor),
    K("expander.background_color",iExpanderBackgroundColor),
    K("expander.disabled_background_color",iExpanderDisabledBackgroundColor),
    K("expander.selected_background_color",iExpanderSelectedBackgroundColor),
    K("expander.selected_active_background_color",iExpanderSelectedActiveBackgroundColor),
    K("expander.active_background_color",iExpanderActiveBackgroundColor),
    K("expander.hover_background_color",iExpanderHoverBackgroundColor),
    K("check.background_color",iCheckBackgroundColor),
    K("check.mark_color",iCheckMarkColor),
    K("icon.folder_tab_color",iFolderTabColor),
    K("icon.folder_body_color",iFolderBodyColor),
    K("icon.file_background_color",iFileBackgroundColor),
    K("icon.file_line_color",iFileLineColor),
    K("scrollbar.track_color",iTrackColor),
    K("scrollbar.thumb_color",iThumbColor),
    K("scrollbar.hover_color",iScrollbarHoverColor),
    K("scrollbar.active_color",iScrollbarActiveColor),
    K("scrollbar.focus_color",iScrollbarFocusColor),
    K("scrollbar.disabled_color",iScrollbarDisabledColor),
};
static void resolve(xui_widget w,void* b,void* e) { __xuiTreeViewResolve(w,b,e); }
int main(void)
{
    xui_widget w; xui_tree_view_data_t* d; xui_tree_view_desc_t desc={0};
    xui_tree_view_node_t nodes[60]={{0}}; int i, work;
    collectionInit();
    for(i=0;i<COUNT(nodes);i++) {
        nodes[i].iId=i+1; nodes[i].iParent=i==1?1:-1; nodes[i].sText="tree node";
        nodes[i].bEnabled=1; nodes[i].bExpanded=1; nodes[i].bCheckReserved=1; nodes[i].bIconReserved=1; nodes[i].bChecked=i&1;
    }
    desc.iSize=sizeof(desc); desc.arrNodes=nodes; desc.iNodeCount=COUNT(nodes); desc.iSelectedId=1;
    CHECK(xuiTreeViewCreate(gContext,&w,&desc)==XUI_OK); d=__xuiTreeViewGetData(w);
    CHECK(xuiTreeViewSetSelected(w,1)==XUI_OK);
    collectionAttach(w); work=gTreeWork;
    collectionVerifyKeys(w,d,sizeof(*d),resolve,keys,COUNT(keys));
    for(i=0;i<COUNT(keys);i++) {
        collectionInline(w,keys[i].sKey,COLOR_C); collectionRender();
        if(strstr(keys[i].sKey,"folder_") || strstr(keys[i].sKey,"file_") ||
           strstr(keys[i].sKey,"check.") || !strcmp(keys[i].sKey,"treeview.text.selected_color") ||
           !strcmp(keys[i].sKey,"treeview.expander.selected_color")) {
            if(!collectionSeen(COLOR_C)) fprintf(stderr,"not rendered: %s\n",keys[i].sKey);
            CHECK(collectionSeen(COLOR_C));
        }
        collectionInline(w,keys[i].sKey,0); collectionRender(); CHECK(!collectionSeen(COLOR_C));
    }
    collectionInline(w,"treeview.scrollbar.thumb_color",COLOR_C); collectionRender(); CHECK(collectionSeen(COLOR_C));
    CHECK(xuiTreeViewSetScrollbarColors(w,COLOR_A,COLOR_D,COLOR_A,COLOR_A,COLOR_A,COLOR_A)==XUI_OK);
    collectionRender(); CHECK(collectionSeen(COLOR_C)); CHECK(!collectionSeen(COLOR_D));
    CHECK(xuiWidgetSetInlineStyle(w,NULL,0)==XUI_OK); collectionRender(); CHECK(collectionSeen(COLOR_D));
    CHECK(gTreeWork==work);
    collectionDone(); puts("style_collections tree: PASS"); return 0;
}
