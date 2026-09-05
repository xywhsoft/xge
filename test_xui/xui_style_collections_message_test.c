#include "../src/xui_internal.h"
static struct { int LayoutNode, MeasureNode, HitNode, RenderNode, HitLine, RenderLine, CaretSource, CaretDecode, CaretCluster, CaretDisplay, HitCaret; } gMessageWork;
#define XUI_MESSAGE_LIST_AUDIT_STEP(name) (++gMessageWork.name)
#include "../src/xui_message_list.c"
#include "xui_style_collections_test.h"
#define K(key, field) {"messagelist." key, offsetof(xui_message_paint_t, tColors)+offsetof(xui_message_list_colors_t, field)}
#define E(key, field) {"messagelist." key, offsetof(xui_message_paint_t, field)}
static const collection_key_t keys[]={
    K("background.color",iBackgroundColor),
    K("bubble.self_color",iSelfBubbleColor),
    K("bubble.other_color",iOtherBubbleColor),
    K("bubble.system_color",iSystemBubbleColor),
    K("text.self_color",iSelfTextColor),
    K("text.other_color",iOtherTextColor),
    K("text.system_color",iSystemTextColor),
    K("text.meta_color",iMetaTextColor),
    K("avatar.self_color",iAvatarSelfColor),
    K("avatar.other_color",iAvatarOtherColor),
    K("row.hover_color",iHoverColor),
    K("row.selected_color",iSelectedColor),
    K("border.color",iBorderColor),
    E("auxiliary.background_color",iAuxiliaryColor),
    E("auxiliary.header_color",iAuxiliaryHeaderColor),
    E("text.selection_color",iTextSelectionColor),
};
static void resolve(xui_widget w,void* b,void* e) { __xuiMessageResolvePaint(w,__xuiMessageListGetData(w),e); }
static int custom(xui_widget w,int index,const xui_message_node_t* node,xui_draw_context draw,xui_rect_t rect,int state,void* user)
{
    if(index!=4) return 0;
    CHECK(gProxy.tProxy.drawRectFill(&gProxy.tProxy,draw,rect,0xe37b21ffu)==XUI_OK);
    return 1;
}
int main(void)
{
    xui_widget w; xui_message_list_data_t* d; xui_message_list_desc_t desc={0};
    xui_message_node_t nodes[5]={{0}}; xui_message_paint_t basePaint;
    xui_message_list_colors_t baseColors, readColors; xui_text_layout layouts[5];
    int i, layoutWork, measureWork;
    collectionInit();
    for(i=0;i<5;i++) { nodes[i].iSize=sizeof(nodes[i]); nodes[i].sText="message content"; nodes[i].sSender="sender"; nodes[i].sTitle="auxiliary"; }
    nodes[0].sId="self"; nodes[0].iType=XUI_MESSAGE_NODE_SELF;
    nodes[1].sId="other"; nodes[1].iType=XUI_MESSAGE_NODE_OTHER;
    nodes[2].sId="system"; nodes[2].iType=XUI_MESSAGE_NODE_SYSTEM;
    nodes[3].sId="aux"; nodes[3].iType=XUI_MESSAGE_NODE_AUXILIARY;
    nodes[4].sId="custom"; nodes[4].iType=XUI_MESSAGE_NODE_OTHER;
    desc.iSize=sizeof(desc); desc.arrNodes=nodes; desc.iNodeCount=5;
    CHECK(xuiMessageListCreate(gContext,&w,&desc)==XUI_OK); d=__xuiMessageListGetData(w);
    CHECK(xuiMessageListSetAutoScroll(w,0)==XUI_OK);
    CHECK(xuiMessageListSetNodeRenderer(w,custom,NULL)==XUI_OK);
    collectionAttach(w);
    d->iHover=1; d->iSelected=0; __xuiMessageSetTextSelection(d,0,0,0,4);
    CHECK(xuiWidgetInvalidate(w,XUI_WIDGET_DIRTY_CACHE|XUI_WIDGET_DIRTY_RENDER)==XUI_OK);
    collectionRender();
    __xuiMessageResolvePaint(w,d,&basePaint); baseColors=d->tColors;
    for(i=0;i<5;i++) layouts[i]=d->arrNodes[i].pTextLayout;
    layoutWork=gMessageWork.LayoutNode; measureWork=gMessageWork.MeasureNode;
    collectionVerifyKeys(w,&basePaint,sizeof(basePaint),resolve,keys,COUNT(keys));
    collectionVerifyRenderedCascade(w,"messagelist.bubble.other_color",baseColors.iOtherBubbleColor);
    for(i=0;i<COUNT(keys);i++) {
        collectionInline(w,keys[i].sKey,COLOR_C); collectionRender();
        if(!collectionSeen(COLOR_C)) fprintf(stderr,"not rendered: %s\n",keys[i].sKey);
        CHECK(collectionSeen(COLOR_C)); CHECK(collectionSeen(0xe37b21ffu));
        collectionInline(w,keys[i].sKey,0); collectionRender(); CHECK(!collectionSeen(COLOR_C));
        CHECK(collectionSeen(0xe37b21ffu));
    }
    collectionInline(w,"messagelist.bubble.self_color",COLOR_C);
    baseColors.iSelfBubbleColor=COLOR_D; CHECK(xuiMessageListSetColors(w,&baseColors)==XUI_OK);
    collectionRender(); CHECK(collectionSeen(COLOR_C)); CHECK(!collectionSeen(COLOR_D));
    CHECK(xuiMessageListGetColors(w,&readColors)==XUI_OK); CHECK(readColors.iSelfBubbleColor==COLOR_D);
    CHECK(xuiWidgetSetInlineStyle(w,NULL,0)==XUI_OK); collectionRender(); CHECK(collectionSeen(COLOR_D));
    CHECK(gMessageWork.LayoutNode==layoutWork); CHECK(gMessageWork.MeasureNode==measureWork);
    for(i=0;i<5;i++) CHECK(d->arrNodes[i].pTextLayout==layouts[i]);
    collectionDone(); puts("style_collections message: PASS"); return 0;
}
