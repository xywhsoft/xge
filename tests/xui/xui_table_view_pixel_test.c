#include "xge.h"
#include "xui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum { WIDTH=160, HEIGHT=96 };
static int started,failed,strict,lines;
static unsigned alpha=255;
static const char* output;
static xui_proxy_t raw;
static int count(xui_widget w,void* u) {(void)w;(void)u;return 2;}
static int cell(xui_widget w,int row,int column,xui_table_view_cell_t* out,void* u) {
    (void)w;(void)row;(void)column;(void)u;out->sText="";return XUI_OK;
}
static int line(xui_proxy p,xui_draw_context d,float x0,float y0,float x1,float y1,float width,uint32_t color) {
    (void)p;++lines;return raw.drawLine(&raw,d,x0,y0,x1,y1,width,color);
}
static int occupied(const unsigned char* pixels,int x,int y,unsigned char background) {
    const unsigned char* p=pixels+4*(y*WIDTH+x);return p[0]!=background || p[1]!=background || p[2]!=background;
}
static int run(void* u) {
    (void)u;if(started)return XGE_OK;started=1;
    xui_context context=NULL;xui_surface target=NULL;xui_widget root=NULL,table=NULL;int began=0;
    unsigned char pixels[WIDTH*HEIGHT*4];xui_rect_t row={0};int horizontal=0,vertical=0;char path[32768];FILE* f=NULL;
#define CHECK(e) do{if(!(e)){fprintf(stderr,"table_view_pixel:%d: %s\n",__LINE__,#e);failed=1;goto cleanup;}}while(0)
    CHECK(xgeBegin()==XGE_OK);began=1;raw=xuiProxyXge();xui_proxy_t proxy=raw;proxy.drawLine=line;
    CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy)==XUI_OK && xuiInputViewport(context,WIDTH,HEIGHT)==XUI_OK);
    CHECK(xuiWidgetCreate(context,&root)==XUI_OK && xuiWidgetSetLayoutType(root,XUI_LAYOUT_MANUAL)==XUI_OK && xuiSetRootWidget(context,root)==XUI_OK);
    xui_table_view_column_t columns[2]={{0}};
    for(int i=0;i<2;++i) {columns[i].iId=i;columns[i].sTitle="";columns[i].fWidth=64;}
    xui_table_view_desc_t desc={0};desc.iSize=sizeof(desc);desc.arrColumns=columns;desc.iColumnCount=2;
    desc.onCount=count;desc.onCell=cell;desc.iSelectionMode=XUI_TABLE_VIEW_SELECTION_ROW;desc.iSelectedRow=-1;
    desc.fDefaultRowHeight=24;desc.fHeaderHeight=24;
    desc.tColors.iBackgroundColor=0x303030ff;desc.tColors.iRowColor=0x303030ff;desc.tColors.iAltRowColor=0x303030ff;
    desc.tColors.iHeaderColor=0x202020ff;desc.tColors.iGridColor=0xd0a04000|alpha;
    CHECK(xuiTableViewCreate(context,&table,&desc)==XUI_OK && xuiWidgetAddChild(root,table)==XUI_OK && xuiWidgetSetRect(table,(xui_rect_t){0,0,WIDTH,HEIGHT})==XUI_OK);
    xui_style_property_t border={0};border.iSize=sizeof(border);border.sName="tableview.border.width";
    border.tValue.iSize=sizeof(border.tValue);border.tValue.iType=XUI_STYLE_VALUE_FLOAT;border.tValue.fFloat=0;
    CHECK(xuiWidgetSetInlineStyle(table,&border,1)==XUI_OK && xuiLayout(context)==XUI_OK && xuiUpdate(context,0)==XUI_OK);
    xui_surface_desc_t surface={XUI_SURFACE_KIND_TEXTURE,XUI_SURFACE_FORMAT_RGBA8,WIDTH,HEIGHT,XUI_SURFACE_USAGE_TARGET|XUI_SURFACE_ALPHA_PREMULTIPLIED};
    CHECK(proxy.surfaceCreate(&proxy,&target,&surface)==XUI_OK && proxy.surfaceClear(&proxy,target,0x303030ff)==XUI_OK);
    xui_rect_i_t clip={0,0,WIDTH,HEIGHT};CHECK(xuiRender(context,target,&clip,1)==XUI_OK && proxy.surfaceReadRGBA(&proxy,target,pixels,WIDTH*4)==XUI_OK);
    CHECK(xuiTableViewGetCellRect(table,0,0,&row)==XUI_OK);
    int edgeX=row.fX+row.fW-1,edgeY=row.fY+row.fH-1;
    CHECK(edgeX>=2 && edgeX+2<WIDTH && edgeY>=2 && edgeY+2<HEIGHT);
    for(int y=edgeY-2;y<=edgeY+2;++y)horizontal+=occupied(pixels,20,y,0x30);
    for(int x=edgeX-2;x<=edgeX+2;++x)vertical+=occupied(pixels,x,12,0x20);
    snprintf(path,sizeof(path),"%s/grid.rgba",output);f=fopen(path,"wb");CHECK(f);CHECK(fwrite(pixels,1,sizeof(pixels),f)==sizeof(pixels));fclose(f);f=NULL;
    snprintf(path,sizeof(path),"%s/pixels.json",output);f=fopen(path,"wb");CHECK(f);
    const unsigned char* body=pixels+4*(edgeY*WIDTH+20),*header=pixels+4*(12*WIDTH+edgeX);
    fprintf(f,"{\"width\":%d,\"height\":%d,\"lineCalls\":%d,\"horizontalPixels\":%d,\"verticalPixels\":%d,\"bodyCenter\":[%u,%u,%u,%u],\"headerCenter\":[%u,%u,%u,%u]}\n",WIDTH,HEIGHT,lines,horizontal,vertical,body[0],body[1],body[2],body[3],header[0],header[1],header[2],header[3]);fclose(f);f=NULL;
    printf("table_view_pixel: body=%dpx header=%dpx antialiased-line-calls=%d\n",horizontal,vertical,lines);
    if(strict)CHECK(horizontal==1 && vertical==1 && lines==0);
cleanup:
    if(f)fclose(f);
    if(context)xuiDestroy(context);
    if(target)raw.surfaceDestroy(&raw,target);
    if(began)xgeEnd();
    xgeQuit();return XGE_OK;
#undef CHECK
}
int main(int argc,char** argv) {
    if(argc<2 || argc>3)return 2;
    output=argv[1];strict=argc==3;
    const char* configured=getenv("XUI_GRID_ALPHA");if(configured){long value=strtol(configured,NULL,10);if(value<1 || value>255)return 2;alpha=(unsigned)value;}
    xge_desc_t desc={0};desc.iWidth=320;desc.iHeight=240;desc.sTitle="Table grid pixel acceptance";desc.iFlags=XGE_INIT_WINDOW;
    int result=xgeInit(&desc);if(result==XGE_OK)result=xgeRun(run,NULL);xgeUnit();return result==XGE_OK && started && !failed?0:1;
}
