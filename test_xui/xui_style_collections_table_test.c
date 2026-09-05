#include "../src/xui_table_view.c"
#include "xui_style_collections_test.h"
#define K(key, field) {"tableview." key, offsetof(xui_table_view_data_t, field)}
static const collection_key_t keys[]={
    K("background.color",iBackgroundColor),
    K("header.color",iHeaderColor),
    K("header.text_color",iHeaderTextColor),
    K("row.color",iRowColor),
    K("row.alt_color",iAltRowColor),
    K("row.hover_color",iHoverColor),
    K("row.selected_color",iSelectedColor),
    K("row.disabled_color",iDisabledColor),
    K("grid.color",iGridColor),
    K("text.color",iTextColor),
    K("text.disabled_color",iDisabledTextColor),
    K("focus.color",iFocusRingColor),
    K("text.selected_color",iSelectedTextColor),
    K("check.background_color",iCheckBackgroundColor),
    K("check.mark_color",iCheckMarkColor),
    K("checker.primary_color",iCheckerPrimaryColor),
    K("checker.secondary_color",iCheckerSecondaryColor),
    K("color.fallback_color",iColorFallbackColor),
    K("cell.invalid_color",iInvalidColor),
    K("cell.dirty_color",iDirtyColor),
    K("cell.editing_color",iEditingColor),
    K("picker.background_color",iPickerBackgroundColor),
    K("picker.border_color",iPickerBorderColor),
    K("scrollbar.track_color",iBarColor),
    K("scrollbar.thumb_color",iThumbColor),
    K("scrollbar.hover_color",iScrollbarHoverColor),
    K("scrollbar.active_color",iScrollbarActiveColor),
    K("scrollbar.focus_color",iScrollbarFocusColor),
    K("scrollbar.disabled_color",iScrollbarDisabledColor),
};
static void resolve(xui_widget w,void* b,void* e) { __xuiTableViewResolve(w,b,e); }
static int cell(xui_widget w,int row,int col,xui_table_view_cell_t* c,void* user)
{
    c->sText=col==0 ? "table row" : (col==1 ? "1" : (col==2 ? "#19375180" : "file"));
    c->iType=col==0 ? XUI_TABLE_CELL_TYPE_TEXT : (col==1 ? XUI_TABLE_CELL_TYPE_BOOL : (col==2 ? XUI_TABLE_CELL_TYPE_COLOR : XUI_TABLE_CELL_TYPE_PICKER));
    c->bInvalid=row==1; c->bDirty=row==2; c->bEditing=row==3;
    if(row==4) { c->bHasStyle=1; c->iTextColor=0xe37b21ffu; c->iBackgroundColor=0x249173ffu; }
    return 1;
}
int main(void)
{
    xui_widget w; xui_widget_type type; xui_widget_type_desc_t td={0};
    xui_table_view_data_t* d; xui_table_view_desc_t desc={0}; xui_table_view_column_t columns[4]={{0}};
    xui_table_view_colors_t base; xui_table_view_row_prefix_t* prefix; uint64_t merges; int i; uint32_t thumb;
    xui_style_property_t p; xui_style_desc_t style={0};
    collectionInit();
    for(i=0;i<4;i++) { columns[i].sTitle="column"; columns[i].fWidth=140; }
    desc.iSize=sizeof(desc); desc.arrColumns=columns; desc.iColumnCount=4; desc.iRowCount=50; desc.onCell=cell;
    td.iSize=sizeof(td); td.sName="collections.derivedtable"; td.pParent=xuiTableViewGetType(gContext);
    td.iTypeDataSize=sizeof(xui_table_view_data_t);
    CHECK(xuiWidgetRegisterType(gContext,&type,&td)==XUI_OK);
    CHECK(xuiWidgetCreateTyped(gContext,type,&w,&desc)==XUI_OK);
    d=__xuiTableViewGetData(w); CHECK(d!=NULL); collectionAttach(w);
    collectionVerifyKeys(w,d,sizeof(*d),resolve,keys,COUNT(keys));
    prefix=d->arrRowPrefix; merges=d->iMergeGeneration;
    collectionVerifyRenderedCascade(w,"tableview.cell.invalid_color",d->iInvalidColor);
    p=collectionProperty("tableview.row.color",COLOR_A); style.iSize=sizeof(style); style.pProperties=&p; style.iPropertyCount=1;
    CHECK(xuiStyleSetType(gContext,xuiTableViewGetType(gContext),&style)==XUI_OK);
    collectionRender(); CHECK(collectionSeen(COLOR_A)); CHECK(collectionSeen(0x249173ffu));
    CHECK(collectionSeen(0xe37b21ffu)); CHECK(d->arrRowPrefix==prefix); CHECK(d->iMergeGeneration==merges);
    p.tValue.iColor=COLOR_B; CHECK(xuiStyleSetType(gContext,xuiTableViewGetType(gContext),&style)==XUI_OK);
    collectionRender(); CHECK(collectionSeen(COLOR_B)); CHECK(!collectionSeen(COLOR_A));
    CHECK(xuiStyleRemoveType(gContext,xuiTableViewGetType(gContext))==XUI_OK); collectionRender();
    for(i=0;i<COUNT(keys);i++) {
        collectionInline(w,keys[i].sKey,COLOR_C); collectionRender();
        /* Non-visible state colors are covered by the effective resolver above. */
        if(strstr(keys[i].sKey,"invalid_color") || strstr(keys[i].sKey,"dirty_color") ||
           strstr(keys[i].sKey,"editing_color") || strstr(keys[i].sKey,"mark_color") ||
           strstr(keys[i].sKey,"picker.") || strstr(keys[i].sKey,"checker.")) {
            if(!collectionSeen(COLOR_C)) fprintf(stderr,"not rendered: %s\n",keys[i].sKey);
            CHECK(collectionSeen(COLOR_C));
        }
        collectionInline(w,keys[i].sKey,0); collectionRender(); CHECK(!collectionSeen(COLOR_C));
    }
    collectionInline(w,"tableview.scrollbar.thumb_color",COLOR_C); collectionRender(); CHECK(collectionSeen(COLOR_C));
    CHECK(xuiTableViewGetColors(w,&base)==XUI_OK); base.iThumbColor=COLOR_D;
    CHECK(xuiTableViewSetColorStyle(w,&base)==XUI_OK); collectionRender(); CHECK(!collectionSeen(COLOR_D));
    CHECK(xuiScrollFrameGetColors(d->pFrame,NULL,&thumb,NULL,NULL,NULL,NULL)==XUI_OK); CHECK(thumb==COLOR_C);
    CHECK(xuiWidgetSetInlineStyle(w,NULL,0)==XUI_OK); collectionRender(); CHECK(collectionSeen(COLOR_D));
    CHECK(d->arrRowPrefix==prefix); CHECK(d->iMergeGeneration==merges);
    collectionDone(); puts("style_collections table: PASS"); return 0;
}
