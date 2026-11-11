/* CPU-only public API test. Linked to actual host/static products, with no
 * implementation inclusion or private field access. The font SVG is provided
 * by the matching lifetime qualification tool. */
#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include "xge.h"
#if defined(XGE_TEST_STATIC)
#include <xge_static_xrt_contract.h>
#else
#include <xge_host_xrt.h>
#include <xge_host_xrt_contract.h>
#endif
#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <string.h>
#if !defined(XGE_TEST_STATIC)
static xge_host_xrt_proc resolve(void* raw,const char* name)
{
    (void)raw;
#define XGE_HOST_XRT_SLOT(symbol) if(!strcmp(name,#symbol))return (xge_host_xrt_proc)&symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
    return NULL;
}
#endif
static char document[65536];
static int documentBytes;
typedef struct State {xge_svg svg;unsigned visits,reads,frees;} State;
static int count(xge_svg_paint paint,void* raw)
{int type=0;assert(xgeSvgPaintTypeGet(paint,&type)==XGE_OK && type);++*(int*)raw;return 1;}
static int traverse(xge_svg_paint paint,void* raw)
{
    State* state=(State*)raw;
    if(!state->visits++){
        int refs=0;assert(xgeSvgRefCountGet(state->svg,&refs)==XGE_OK && refs==2);
        assert(xrtMemDebugFailAfter(0));
        assert(xgeSvgClear(state->svg)==XGE_ERROR_BUSY);
        assert(xgeSvgLoadMemory(state->svg,document,documentBytes)==XGE_ERROR_BUSY);
        assert(xgeSvgLoad(state->svg,"svg-public://blocked")==XGE_ERROR_BUSY);
        assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();
        int nested=0;assert(xgeSvgPaintTraverse(paint,count,&nested)==XGE_OK && nested>1);
    }
    return 1;
}
static int readDocument(const xstrview* path,xge_resource_output_t* output,void* raw)
{
    State* state=(State*)raw;assert(path && path->Data);++state->reads;
    assert(xgeSvgClear(state->svg)==XGE_ERROR_BUSY);
    xge_svg clone=(xge_svg)(uintptr_t)1;assert(xgeSvgClone(state->svg,&clone)==XGE_ERROR_BUSY && !clone);
    int refs=0;assert(xgeSvgRefCountGet(state->svg,&refs)==XGE_OK && refs==2);
    xgeSvgDestroy(state->svg);
    return xgeResourceOutputSet(output,document,(size_t)documentBytes);
}
static int readLegacy(const char* uri,void** data,int* bytes,void* raw)
{
    (void)uri;State* state=(State*)raw;++state->reads;
    *data=xrtMalloc((size_t)documentBytes);if(!*data)return XGE_ERROR_OUT_OF_MEMORY;
    memcpy(*data,document,(size_t)documentBytes);*bytes=documentBytes;return XGE_OK;
}
static void freeLegacy(void* data,void* raw)
{
    State* state=(State*)raw;++state->frees;assert(xgeSvgClear(state->svg)==XGE_ERROR_BUSY);
    int refs=0;assert(xgeSvgRefCountGet(state->svg,&refs)==XGE_OK && refs==2);
    xgeSvgDestroy(state->svg);xrtFree(data);
}
int main(int argc,char** argv)
{
#if defined(XGE_TEST_STATIC)
    const xge_host_xrt_contract_t contract=XGE_STATIC_XRT_LOCAL_CONTRACT_INIT;
    assert(contract.revision==xgeStaticXrtContractV1.revision && contract.dataModel==xgeStaticXrtContractV1.dataModel &&
        contract.coreLayout==xgeStaticXrtContractV1.coreLayout && !strcmp(contract.headerSha256,xgeStaticXrtContractV1.headerSha256));
#else
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
#endif
    assert(argc==2);FILE* file=fopen(argv[1],"rb");assert(file);
    size_t bytes=fread(document,1,sizeof(document)-1,file);assert(bytes && bytes<sizeof(document)-1 && !ferror(file));
    assert(!fclose(file));documentBytes=(int)bytes;document[bytes]=0;
    assert(xrtMemDebugEnable(true));xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    xge_svg svg=NULL;assert(xgeSvgCreate(&svg)==XGE_OK);
    assert(xgeSvgLoadMemory(svg,document,documentBytes)==XGE_OK);
    xge_svg_paint picture=NULL,child=NULL,parent=NULL;int children=0;
    assert(xgeSvgPaintGetPicture(svg,&picture)==XGE_OK);
    assert(xgeSvgPaintChildGetCount(picture,&children)==XGE_OK && children==1);
    assert(xgeSvgPaintChildGetAt(picture,0,&child)==XGE_OK);
    assert(xgeSvgPaintParentGet(child,&parent)==XGE_OK && parent==picture);
    State visit={svg,0,0,0};assert(xgeSvgPaintTraverse(picture,traverse,&visit)==XGE_OK && visit.visits>1);
    assert(xgeSvgClear(svg)==XGE_OK);assert(xgeSvgLoadMemory(svg,document,documentBytes)==XGE_OK);
    xge_svg clone=NULL;assert(xgeSvgClone(svg,&clone)==XGE_OK && clone);
    xgeSvgDestroy(svg);xgeSvgDestroy(clone);
    assert(xgeSvgCreate(&svg)==XGE_OK);State read={svg,0,0,0};
    assert(xgeResourceProviderAddOwned("svg-public",readDocument,&read,NULL)==XGE_OK);
    assert(xgeSvgLoad(svg,"svg-public://source")==XGE_OK && read.reads==1);
    assert(xgeResourceProviderClearChecked()==XGE_OK);
    assert(xgeSvgCreate(&svg)==XGE_OK);State release={svg,0,0,0};
    xge_resource_provider_t provider={"svg-public",readLegacy,freeLegacy,&release};
    assert(xgeResourceProviderAdd(&provider)==XGE_OK);
    assert(xgeSvgLoad(svg,"svg-public://source")==XGE_OK && release.reads==1 && release.frees==1);
    assert(xgeResourceProviderClearChecked()==XGE_OK);xrtClearError();xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    assert(before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
    puts("Actual SVG product: tree, reentry, owner drop, clone, provider return tail and heap equality passed");return 0;
}
