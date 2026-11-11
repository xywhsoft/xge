/* Authoritative SVG implementation and real project-owned embedded font.
 * Old controls stop at assertions before an invalidated paint is accessed. */
#define XRT_MODULE_ALL
#include "single/xge.c"
#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static char document[65536];
static int documentBytes;
static void equalHeap(const xmemdebugsnapshot* a,const xmemdebugsnapshot* b)
{
    assert(a->LiveCount==b->LiveCount && a->LiveBytes==b->LiveBytes);
    assert(a->InvalidFreeCount==b->InvalidFreeCount && a->DoubleFreeCount==b->DoubleFreeCount);
}
static xge_svg create(void)
{
    xge_svg svg=NULL;assert(xgeSvgCreate(&svg)==XGE_OK && svg);
    assert(xgeSvgLoadMemory(svg,document,documentBytes)==XGE_OK);
    assert(svg->iFontFaceCount==2);
    return svg;
}
typedef struct Visit {xge_svg svg;int mode,visits,nested,released;} Visit;
static int countPaint(xge_svg_paint paint,void* raw)
{
    int type=0;assert(xgeSvgPaintTypeGet(paint,&type)==XGE_OK && type);
    ++*(int*)raw;return 1;
}
static int visit(xge_svg_paint paint,void* raw)
{
    Visit* state=raw;++state->visits;
    if(state->visits!=1)return 1;
    if(state->mode==2){
        assert(xgeSvgClear(state->svg)==XGE_ERROR_BUSY);
    }else if(state->mode==3){
        assert(xrtMemDebugFailAfter(0));
        assert(xgeSvgLoadMemory(state->svg,document,documentBytes)==XGE_ERROR_BUSY);
        assert(xgeSvgLoad(state->svg,"svg-local://blocked")==XGE_ERROR_BUSY);
        assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();
    }else if(state->mode==4){
        assert(state->svg->iRefCount>=2);
        xgeSvgDestroy(state->svg);state->released=1;
        int type=0;assert(xgeSvgPaintTypeGet(paint,&type)==XGE_OK && type);
    }
#if !defined(XGE_TEST_SVG_LIFETIME_BASELINE)
    else if(state->mode==6){
        assert(xgeSvgPaintTraverse(paint,countPaint,&state->nested)==XGE_OK && state->nested>1);
        assert(xgeSvgPaintOpacitySet(paint,0.5f)==XGE_OK);
    }else if(state->mode==7){
        xge_shape_ex shape=NULL;assert(xgeShapeExCreate(&shape)==XGE_OK);
        assert(xgeShapeExAppendRect(shape,0,0,10,10,0,0,1)==XGE_OK);
        assert(xgeSvgPaintMaskShapeSet(paint,shape,XGE_SHAPE_EX_MASK_ALPHA)==XGE_ERROR_BUSY);
        assert(xgeSvgPaintMaskClear(paint)==XGE_ERROR_BUSY);
        assert(xgeSvgPaintClipShapeSet(paint,shape)==XGE_ERROR_BUSY);
        assert(xgeSvgPaintClipClear(paint)==XGE_ERROR_BUSY);
        xge_shape_ex_scene scene=NULL;assert(xgeShapeExSceneCreate(&scene)==XGE_OK);
        assert(xgeSvgPaintMaskSceneSet(paint,scene,XGE_SHAPE_EX_MASK_ALPHA)==XGE_ERROR_BUSY);
        xgeShapeExSceneDestroy(scene);
        xgeShapeExDestroy(shape);
    }
#endif
    return 1;
}
typedef struct Provider {xge_svg svg;unsigned calls;int release;} Provider;
static int provide(const xstrview* path,xge_resource_output_t* output,void* raw)
{
    Provider* state=raw;assert(path && path->Data);++state->calls;
    assert(xgeSvgClear(state->svg)==XGE_ERROR_BUSY);
#if !defined(XGE_TEST_SVG_LIFETIME_BASELINE)
    assert(xgeSvgLoadMemory(state->svg,document,documentBytes)==XGE_ERROR_BUSY);
    assert(xgeSvgLoad(state->svg,"svg-local://recursive")==XGE_ERROR_BUSY);
    xge_svg_paint picture=NULL;assert(xgeSvgPaintGetPicture(state->svg,&picture)==XGE_OK);
    int count=0;assert(xgeSvgPaintTraverse(picture,countPaint,&count)==XGE_ERROR_BUSY && !count);
#endif
    if(state->release){
        assert(state->svg->iRefCount>=2);xgeSvgDestroy(state->svg);
    }
    return xgeResourceOutputSet(output,document,(size_t)documentBytes);
}
static int stop(xge_svg_paint paint,void* raw)
{(void)paint;++*(int*)raw;return 0;}
static int supply(const xstrview* path,xge_resource_output_t* output,void* raw)
{(void)path;(void)raw;return xgeResourceOutputSet(output,document,(size_t)documentBytes);}
typedef struct Legacy {xge_svg svg;unsigned loads,frees;} Legacy;
static int legacyLoad(const char* uri,void** output,int* size,void* raw)
{
    (void)uri;Legacy* state=raw;++state->loads;
    *output=xrtMalloc((size_t)documentBytes);if(!*output)return XGE_ERROR_OUT_OF_MEMORY;
    memcpy(*output,document,(size_t)documentBytes);*size=documentBytes;return XGE_OK;
}
static void legacyFree(void* data,void* raw)
{
    Legacy* state=raw;++state->frees;
    assert(xgeSvgClear(state->svg)==XGE_ERROR_BUSY && state->svg->iRefCount>=2);
    xgeSvgDestroy(state->svg);xrtFree(data);
}

int main(int argc,char** argv)
{
    assert(argc==3);int mode=atoi(argv[1]);assert(mode>=0 && mode<=16);
    FILE* input=fopen(argv[2],"rb");assert(input);
    size_t bytes=fread(document,1,sizeof(document)-1,input);assert(bytes>0 && bytes<sizeof(document)-1 && !ferror(input));
    assert(!fclose(input));document[bytes]=0;documentBytes=(int)bytes;
    assert(xrtMemDebugEnable(true));xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    if(mode==0){
        xge_svg svg=create();xgeSvgDestroy(svg);
    }else if(mode==1){
        xge_svg svg=create();svg->iRefCount=0;
        assert(xgeSvgAddRef(svg)==XGE_ERROR_INVALID_STATE && svg->iRefCount==0);
        xgeSvgDestroy(svg);assert(svg->iMagic==XGE_SVG_MAGIC);
        svg->iRefCount=1;xgeSvgDestroy(svg);
    }else if(mode>=2 && mode<=4){
        xge_svg svg=create();xge_svg_paint picture=NULL;assert(xgeSvgPaintGetPicture(svg,&picture)==XGE_OK);
        Visit state={svg,mode,0,0,0};assert(xgeSvgPaintTraverse(picture,visit,&state)==XGE_OK && state.visits>1);
        if(!state.released){assert(svg->iRefCount==1);xgeSvgDestroy(svg);}
    }else if(mode==5){
        xge_svg svg=create();Provider state={svg,0,1};
        assert(xgeResourceProviderAddOwned("svg-local",provide,&state,NULL)==XGE_OK);
        assert(xgeSvgLoad(svg,"svg-local://source")==XGE_OK && state.calls==1);
        assert(xgeResourceProviderClearChecked()==XGE_OK);
    }else if(mode==6){
        xge_svg svg=create();xge_svg_paint picture=NULL,child=NULL,parent=(void*)1;
        assert(xgeSvgPaintGetPicture(svg,&picture)==XGE_OK);int count=0;
        assert(xgeSvgPaintChildGetCount(picture,&count)==XGE_OK && count==1);
        assert(xgeSvgPaintChildGetAt(picture,0,&child)==XGE_OK && child);
        assert(xgeSvgPaintParentGet(child,&parent)==XGE_OK && parent==picture);
        assert(xgeSvgPaintParentGet(picture,&parent)==XGE_OK && !parent);
        Visit state={svg,mode,0,0,0};assert(xgeSvgPaintTraverse(picture,visit,&state)==XGE_OK && state.visits>1);
        assert(xgeSvgClear(svg)==XGE_OK);xgeSvgDestroy(svg);
    }
#if !defined(XGE_TEST_SVG_LIFETIME_BASELINE)
    else if(mode==7){
        xge_svg svg=create();xge_svg_paint picture=NULL;assert(xgeSvgPaintGetPicture(svg,&picture)==XGE_OK);
        Visit state={svg,mode,0,0,0};assert(xgeSvgPaintTraverse(picture,visit,&state)==XGE_OK && state.visits>1);
        assert(!svg->iActiveTraversals && !svg->bLoading && svg->iRefCount==1);
        int count=0;assert(xgeSvgPaintTraverse(picture,stop,&count)==XGE_OK && count==1);
        assert(xgeSvgClear(svg)==XGE_OK && !svg->iFontFaceCount);xgeSvgDestroy(svg);
    }else if(mode==8){
        /* Every prefix is checked against a still-live source document. */
        xge_svg source=create();unsigned failures=0,successes=0;
        xmemdebugsnapshot resident;xrtMemDebugSnapshot(&resident);
        for(unsigned budget=0;budget<96;++budget){
            assert(xrtMemDebugFailAfter((int64_t)budget));xge_svg clone=(void*)1;
            int status=xgeSvgClone(source,&clone);xrtMemDebugFailClear();
            if(status==XGE_OK){assert(clone && clone->iFontFaceCount==2);xgeSvgDestroy(clone);++successes;}
            else{assert(status==XGE_ERROR_OUT_OF_MEMORY && !clone);++failures;}
            xrtClearError();xrtMemDebugSnapshot(&after);equalHeap(&resident,&after);
            assert(source->iRefCount==1 && !source->bLoading && !source->iActiveTraversals);
        }
        assert(failures && successes);printf("SVG clone allocation prefixes: %u failed, %u passed\n",failures,successes);
        xgeSvgDestroy(source);
    }else if(mode==9){
        xge_svg svg=create();svg->iRefCount=INT_MAX;
        assert(xgeSvgAddRef(svg)==XGE_ERROR_OUT_OF_MEMORY && svg->iRefCount==INT_MAX);
        svg->iRefCount=1;svg->iActiveTraversals=UINT32_MAX;
        xge_svg_paint picture=NULL;assert(xgeSvgPaintGetPicture(svg,&picture)==XGE_OK);
        int count=0;assert(xgeSvgPaintTraverse(picture,countPaint,&count)==XGE_ERROR_OUT_OF_MEMORY && !count && svg->iRefCount==1);
        svg->iActiveTraversals=0;xgeSvgDestroy(svg);
    }else if(mode==10){
        xge_svg svg=create();Provider state={svg,0,0};
        assert(xgeResourceProviderAddOwned("svg-local",provide,&state,NULL)==XGE_OK);
        assert(xgeSvgLoad(svg,"svg-local://source")==XGE_OK && state.calls==1);
        assert(svg->iRefCount==1 && !svg->bLoading && !svg->iActiveTraversals);
        xge_rect_t bounds={0};assert(xgeSvgGetBounds(svg,0.25f,&bounds)==XGE_OK && bounds.fW>0 && bounds.fH>0);
        assert(xgeSvgClear(svg)==XGE_OK);assert(xgeSvgLoadMemory(svg,document,documentBytes)==XGE_OK);
        assert(xgeResourceProviderClearChecked()==XGE_OK);xgeSvgDestroy(svg);
    }else if(mode==11){
        assert(xgeResourceProviderAddOwned("svg-local",supply,NULL,NULL)==XGE_OK);
        xge_svg svg=NULL;assert(xgeSvgLoadCached("svg-local://cached",&svg)==XGE_OK && svg && svg->iRefCount==2);
        svg->iRefCount=INT_MAX;xge_svg second=(void*)1;
        assert(xgeSvgLoadCached("svg-local://cached",&second)==XGE_ERROR_OUT_OF_MEMORY && second==(void*)1 && svg->iRefCount==INT_MAX);
        svg->iRefCount=2;xgeSvgDestroy(svg);
        assert(xgeSvgCacheInvalidate("svg-local://cached")==XGE_OK);
        assert(xgeResourceProviderClearChecked()==XGE_OK);
    }else if(mode==12){
        xge_svg parent=create(),child=create();int count=parent->iItemCount;
        xge_svg_image_item_t image={0};image.pSvg=child;image.tRect=(xge_rect_t){0,0,10,10};
        xge_svg_style_t style=__xgeSvgStyleDefault();xge_svg_def_t definition={0};child->iRefCount=INT_MAX;
        assert(__xgeSvgAddImageItemCopy(parent,&image,-1)==XGE_ERROR_OUT_OF_MEMORY);
        assert(child->iRefCount==INT_MAX && parent->iItemCount==count);
        assert(__xgeSvgDefAddImage(&definition,&image)==XGE_ERROR_OUT_OF_MEMORY);
        assert(child->iRefCount==INT_MAX && !definition.iImageCount);
        assert(__xgeSvgAddImageItem(parent,&style,child,image.tRect)==XGE_ERROR_OUT_OF_MEMORY);
        assert(child->iRefCount==INT_MAX && parent->iItemCount==count);
        xrtFree(definition.pImages);child->iRefCount=1;xgeSvgDestroy(child);xgeSvgDestroy(parent);
    }else if(mode==13){
        xge_svg svg=NULL;assert(xgeSvgCreate(&svg)==XGE_OK);unsigned failures=0,successes=0;
        xmemdebugsnapshot resident;xrtMemDebugSnapshot(&resident);
        for(unsigned budget=0;budget<128;++budget){
            assert(xrtMemDebugFailAfter((int64_t)budget));
            int status=xgeSvgLoadMemory(svg,document,documentBytes);xrtMemDebugFailClear();
            if(status==XGE_OK)++successes;else{assert(status==XGE_ERROR_OUT_OF_MEMORY);++failures;}
            assert(svg->iRefCount==1 && !svg->bLoading && !svg->iActiveTraversals);
            assert(xgeSvgClear(svg)==XGE_OK);xrtClearError();xrtMemDebugSnapshot(&after);equalHeap(&resident,&after);
        }
        assert(failures && successes);printf("SVG load allocation prefixes: %u failed, %u passed\n",failures,successes);
        xgeSvgDestroy(svg);
    }else if(mode==14){
        xge_svg svg=create();xge_font_face face=__xgeSvgFontFaceEnsure(&svg->pFontFaces[0]);
        assert(face && !svg->pFontFaces[0].pData && !svg->pFontFaces[0].iSize);
        assert(xgeFontFaceAddRef(face)==2);xgeSvgDestroy(svg);
        xge_font_face_info_t info={0};info.iSize=sizeof(info);
        assert(xgeFontFaceGetInfo(face,&info)==XGE_OK);xgeFontFaceFree(face);
    }else if(mode==15){
        xge_svg svg=create();Legacy state={svg,0,0};
        xge_resource_provider_t provider={"svg-legacy",legacyLoad,legacyFree,&state};
        assert(xgeResourceProviderAdd(&provider)==XGE_OK);
        assert(xgeSvgLoad(svg,"svg-legacy://source")==XGE_OK && state.loads==1 && state.frees==1);
        assert(xgeResourceProviderClearChecked()==XGE_OK);
    }else if(mode==16){
        static const char* vectors[]={"","TQ==","TWE=","TWFu"," Zm9v\r\nYmFy ","/////////////////////w=="};
        static const char* expected[]={"","M","Ma","Man","foobar",NULL};
        static const int sizes[]={0,1,2,3,6,16};
        for(unsigned i=0;i<sizeof(vectors)/sizeof(*vectors);++i){
            unsigned char* data=NULL;int size=-1;
            assert(__xgeSvgDecodeBase64(vectors[i],&data,&size)==XGE_OK && data && size==sizes[i]);
            if(expected[i])assert(!memcmp(data,expected[i],(size_t)size));
            else for(int j=0;j<size;++j)assert(data[j]==255);
            xrtFree(data);
        }
        unsigned char* data=(void*)1;int size=-1;
        assert(__xgeSvgDecodeBase64("AA$A",&data,&size)==XGE_ERROR_INVALID_ARGUMENT && !data && !size);
        assert(xrtMemDebugFailAfter(0));data=(void*)1;size=-1;
        assert(__xgeSvgDecodeBase64("TWFu",&data,&size)==XGE_ERROR_OUT_OF_MEMORY && !data && !size);
        xrtMemDebugFailClear();xrtClearError();
    }
#endif
    xrtClearError();xrtMemDebugSnapshot(&after);equalHeap(&before,&after);
    printf("SVG lifetime mode %d passed; private document %zu bytes\n",mode,sizeof(struct xge_svg_t));return 0;
}
