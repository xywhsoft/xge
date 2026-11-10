/* Font ownership and public numeric boundaries. Inputs are generated valid
 * fonts. An explicit EGL SDK also checks real queued drawing. No network,
 * window, hardware performance, malformed-font fuzzing or audio test. */
#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include <xge.h>
#if defined(XGE_TEST_STATIC)
#include <xge_static_xrt_contract.h>
#else
#include <xge_host_xrt.h>
#include <xge_host_xrt_contract.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#if !defined(XGE_TEST_STATIC)
static xge_host_xrt_proc resolve(void* user,const char* name)
{
 (void)user;
#define XGE_HOST_XRT_SLOT(symbol) if(!strcmp(name,#symbol))return (xge_host_xrt_proc)&symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
 return NULL;
}
#endif
static unsigned u16(const unsigned char* p){return ((unsigned)p[0]<<8)|p[1];}
static unsigned u32(const unsigned char* p){return ((unsigned)p[0]<<24)|((unsigned)p[1]<<16)|((unsigned)p[2]<<8)|p[3];}
static void attributes(unsigned char* data,size_t size,unsigned weight,unsigned stretch)
{
 assert(size>=12);unsigned tables=u16(data+4);assert(tables<=(size-12)/16);
 for(unsigned i=0;i<tables;++i){unsigned char* entry=data+12+16*i;
  if(!memcmp(entry,"OS/2",4)){
   size_t at=u32(entry+8),bytes=u32(entry+12);assert(at<=size && bytes>=8 && bytes<=size-at);
   data[at+4]=(unsigned char)(weight>>8);data[at+5]=(unsigned char)weight;
   data[at+6]=(unsigned char)(stretch>>8);data[at+7]=(unsigned char)stretch;return;
  }
 }
 assert(!"fixture must contain OS/2 metadata");
}
static int64_t score(int weight,int stretch,int expectedWeight,int expectedStretch)
{if(weight<=0)weight=XGE_FONT_WEIGHT_NORMAL;if(stretch<=0)stretch=XGE_FONT_STRETCH_NORMAL;return llabs((int64_t)weight-expectedWeight)+4*llabs((int64_t)stretch-expectedStretch);}
static unsigned providerCalls;
static int failingProvider(const xstrview* path,xge_resource_output_t* output,void* user)
{
 (void)path;(void)output;(void)user;++providerCalls;
 xrtSetErrorTake(xrtErrorCreate(XERR_STATE,"font-provider",71,"required provider failure"));return XGE_ERROR_CALLBACK_FAILED;
}
int main(int argc,char** argv)
{
 assert(argc==5);unsigned mode=(unsigned)atoi(argv[2]);assert(mode<=9);
#if !defined(XGE_TEST_STATIC)
 const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_OK);
#endif
 assert(xrtMemDebugEnable(true));assert(xgeInit(NULL)==XGE_OK && xgeUnitChecked()==XGE_OK);
 xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
 xge_desc_t engine={0};engine.iRunMode=XGE_RUN_MANUAL;assert(xgeInit(&engine)==XGE_OK);
 FILE* file=fopen(argv[1],"rb");assert(file && !fseek(file,0,SEEK_END));long length=ftell(file);assert(length>0 && length<INT_MAX);rewind(file);
 unsigned char* data=xrtMalloc((size_t)length);assert(data && fread(data,1,(size_t)length,file)==(size_t)length && !fclose(file));
 attributes(data,(size_t)length,400,5);xge_font_face regular=NULL,bold=NULL;
 assert(xgeFontFaceLoadMemory(&regular,data,(int)length,NULL)==XGE_OK);
 attributes(data,(size_t)length,700,9);assert(xgeFontFaceLoadMemory(&bold,data,(int)length,NULL)==XGE_OK);
 xge_font_face_info_t ri={0},bi={0};assert(xgeFontFaceGetInfo(regular,&ri)==XGE_OK && ri.iWeight==400 && ri.iStretch==XGE_FONT_STRETCH_NORMAL);
 assert(xgeFontFaceGetInfo(bold,&bi)==XGE_OK && bi.iWeight==700 && bi.iStretch==200);
 xge_font_family family=NULL;assert(xgeFontFamilyCreate(&family)==XGE_OK);
 assert(xgeFontFamilyAddFace(family,regular)==XGE_OK && xgeFontFamilyAddFace(family,bold)==XGE_OK);
 if(mode==0){
  const int weights[]={400,700,INT_MAX-1000,INT_MAX,INT_MIN},widths[]={1,100,200,500,INT_MAX,INT_MIN};
  for(unsigned i=0;i<sizeof(weights)/sizeof(*weights);++i)for(unsigned j=0;j<sizeof(widths)/sizeof(*widths);++j){
   xge_font_face face=NULL;int status=xgeFontFamilyResolveEx(family,weights[i],widths[j],0,&face);
   xge_font_face expected=score(weights[i],widths[j],ri.iWeight,ri.iStretch)<=score(weights[i],widths[j],bi.iWeight,bi.iStretch) ? regular:bold;
   if(status!=XGE_OK || face!=expected){fprintf(stderr,"Font match weight=%d stretch=%d status=%d wrong=%d\n",weights[i],widths[j],status,face!=expected);return 10;}
   xgeFontFaceFree(face);
  }
 }else if(mode==1){
  const float values[]={NAN,INFINITY,-INFINITY,0,-1};
  for(unsigned i=0;i<sizeof(values)/sizeof(*values);++i){
   xge_font_t font={0};xge_font_instance_desc_t desc={sizeof(desc),values[i],0};
   assert(xgeFontCreate(&font,regular,&desc)==XGE_ERROR_INVALID_ARGUMENT && !font.iRefCount);
   assert(xgeFontLoadMemory(&font,data,(int)length,values[i])==XGE_ERROR_INVALID_ARGUMENT && !font.iRefCount);
   assert(xgeFontLoad(&font,"must-not-be-opened",values[i])==XGE_ERROR_INVALID_ARGUMENT);
   assert(xgeFontLoadCached(&font,"must-not-be-opened","must-not-be-opened",values[i],65,1)==XGE_ERROR_INVALID_ARGUMENT);
  }
 }else if(mode==3 || mode==4){
  xge_font_t font={0};xge_font_instance_desc_t desc={sizeof(desc),mode==4 ? FLT_MAX:20,0};
  assert(xgeFontCreate(&font,regular,&desc)==XGE_OK);
  xge_glyph_metrics_t glyph={0};xge_glyph_bitmap_t bitmap={0};
  if(mode==3){
   assert(xgeFontGlyphGetByIndex(&font,INT_MAX,&glyph)==XGE_ERROR_INVALID_ARGUMENT);
   assert(xgeFontGlyphRasterizeByIndex(&font,INT_MAX,&bitmap)==XGE_ERROR_INVALID_ARGUMENT && !bitmap.pPixels);
  }else{
   int result=xgeFontGlyphGet(&font,65,&glyph);
   fprintf(stderr,"Large glyph result=%d scale=%g\n",result,(double)font.fScale);
   assert(result==XGE_ERROR_OUT_OF_MEMORY);
   assert(xgeFontGlyphRasterize(&font,65,&bitmap)==XGE_ERROR_OUT_OF_MEMORY && !bitmap.pPixels);
  }
  xgeFontFree(&font);
 }else if(mode==7){
  assert(xgeFontFallbackSetMemory(data,(int)length,20)==XGE_OK);
  xge_font_t font={0};assert(xgeFontLoad(&font,"missing-font-for-fallback",18)==XGE_OK && !xrtGetError());xgeFontFree(&font);
  xerror* ambient=xrtErrorCreate(XERR_STATE,"font-caller",72,"caller diagnostic");assert(ambient);xrtSetErrorTake(ambient);
  assert(xgeFontLoad(&font,"missing-font-for-fallback",18)==XGE_OK && xrtGetError()==ambient);xgeFontFree(&font);xrtClearError();
  assert(xgeResourceProviderAddOwned("fontcheck",failingProvider,NULL,NULL)==XGE_OK);
  assert(xgeFontLoad(&font,"fontcheck://input",18)==XGE_ERROR_CALLBACK_FAILED && providerCalls==1 && !font.iRefCount);
  assert(xrtErrorCode(xrtGetError())==71);xrtClearError();
  assert(xgeFontLoadCached(&font,argv[1],"fontcheck://cache",20,65,2)==XGE_ERROR_CALLBACK_FAILED && providerCalls==2 && !font.iRefCount);
  assert(xrtErrorCode(xrtGetError())==71);xrtClearError();
  assert(xgeResourceProviderClearChecked()==XGE_OK);xgeFontFallbackClear();
 }else if(mode==8){
  xge_font_t font={0};xge_font_instance_desc_t desc={sizeof(desc),20,0};assert(xgeFontCreate(&font,regular,&desc)==XGE_OK);
  unsigned failures=0,successes=0;
  for(int64_t budget=0;budget<32;++budget){
   xgeFontCacheClear(&font);void* output=(void*)(uintptr_t)1;int bytes=-1;
   assert(xrtMemDebugFailAfter(budget));int status=xgeFontBuildXRFMemory(&font,65,2,&output,&bytes);
   bool triggered=xrtMemDebugFailTriggered();xrtMemDebugFailClear();
   if(status==XGE_OK){assert(!triggered && output && bytes>0);++successes;xrtFree(output);}
   else{assert(status==XGE_ERROR_OUT_OF_MEMORY && triggered && !output && !bytes);++failures;}
   xrtClearError();
  }
  assert(failures && successes);xgeFontFree(&font);
 }else if(mode==5){
  assert(xgeFontFallbackSetMemory(data,(int)length,18)==XGE_OK);
  for(int64_t budget=0;budget<3;++budget){
   assert(xrtMemDebugFailAfter(budget));int status=xgeFontFallbackSetMemory(data,(int)length,24);
   assert(status==XGE_ERROR_OUT_OF_MEMORY && xrtMemDebugFailTriggered());xrtMemDebugFailClear();xrtClearError();
   xge_font_t font={0};assert(xgeFontFallbackGet(&font,18)==XGE_OK);xgeFontFree(&font);
  }
  assert(xgeFontFallbackSetMemory(data,(int)length,NAN)==XGE_ERROR_INVALID_ARGUMENT);
  xge_font_t font={0};assert(xgeFontFallbackGet(&font,18)==XGE_OK);xgeFontFree(&font);xgeFontFallbackClear();
 }
#if defined(XGE_FONT_TEST_OWNED)
 else if(mode==9){
  xge_graphics_context context=NULL;int status=xgeGraphicsContextCreateEglOwned(&context,64,64,true,NULL);
#if XGE_ENABLE_EGL
  assert(status==XGE_OK && context);
  xge_font font=NULL;xge_font_instance_desc_t desc={sizeof(desc),20,0};assert(xgeFontCreateOwned(&font,regular,&desc)==XGE_OK);
  xge_render_target target=NULL;xge_pass pass=NULL;assert(xgeRenderTargetCreateOwned(&target,64,64)==XGE_OK);
  assert(xgePassCreateOwned(&pass,target,XGE_PASS_CLEAR_COLOR,XGE_COLOR_RGBA(0,0,0,255))==XGE_OK && xgePassBegin(pass)==XGE_OK);
  assert(xgeGraphicsOwnedResourceCount()==3); /* Pass shares the target root. */
  xgeTextDraw(font,"AV",4,24,XGE_COLOR_RGBA(255,255,255,255));
  xgeFontCacheClear(font); /* Queued draws retain stable atlas records. */
  xgeTextDraw(font,"ffi",4,48,XGE_COLOR_RGBA(255,255,255,255));
  xgeFontFree(font); /* Both pending draws must survive the font itself. */
  assert(xgeGraphicsOwnedResourceCount()==2);
  assert(xgePassEnd(pass)==XGE_OK);
  unsigned char pixels[64*64*4];assert(xgeRenderTargetReadPixelsSized(target,pixels,sizeof(pixels),64*4)==XGE_OK);
  unsigned painted=0;for(unsigned i=0;i<64*64;++i)if(pixels[i*4] || pixels[i*4+1] || pixels[i*4+2])++painted;
  assert(painted>20 && painted<64*64);xgePassFree(pass);xgeRenderTargetFree(target);
  assert(xgeGraphicsOwnedResourceCount()==1);
  bool consumed=false;assert(xgeGraphicsContextReleaseOwnerChecked(context,&consumed)==XGE_OK && consumed);
  assert(!xgeGraphicsOwnedResourceCount());
  printf("Actual EGL queued font rendering: %u painted pixels after clear/free\n",painted);
#else
  assert(status==XGE_ERROR_UNSUPPORTED && !context);
#endif
 }
 else if(mode==2 || mode==6){
  xge_font font=NULL;xge_font_instance_desc_t desc={sizeof(desc),20,0};
  assert(xgeFontCreateOwned(&font,regular,&desc)==XGE_OK && xgeFontIsOwned(font) && xgeGraphicsOwnedResourceCount()==1);
  xgeFontFaceFree(regular);regular=NULL;
  xge_font_metrics_t metrics={0};assert(xgeFontGetMetrics(font,&metrics)==XGE_OK && metrics.fPixelSize==20);
  if(mode==2){
   for(int64_t budget=0;budget<3;++budget){
    xge_font copy=(xge_font)(uintptr_t)1;assert(xrtMemDebugFailAfter(budget));
    int status=xgeFontCreateOwned(&copy,bold,&desc);xrtMemDebugFailClear();xrtClearError();
    assert((status==XGE_OK)==(copy!=NULL));xgeFontFree(copy);assert(xgeGraphicsOwnedResourceCount()==1);
   }
   xge_font_face missing=(xge_font_face)(uintptr_t)1;
   assert(xgeFontFaceLoadMemory(&missing,NULL,0,NULL)==XGE_ERROR_INVALID_ARGUMENT && !missing);
   xge_font_family empty=NULL;assert(xgeFontFamilyCreate(&empty)==XGE_OK);
   missing=(xge_font_face)(uintptr_t)1;assert(xgeFontFamilyResolveEx(empty,400,5,0,&missing)==XGE_ERROR_RESOURCE_FAILED && !missing);
   xgeFontFamilyFree(empty);
   assert(xgeFontFallbackSetMemory(data,(int)length,20)==XGE_OK);
   xge_font fallback=NULL;assert(xgeFontFallbackGetOwned(&fallback,22,0)==XGE_OK && xgeFontIsOwned(fallback));
   xgeFontFallbackClear();assert(xgeFontGetMetrics(fallback,&metrics)==XGE_OK && metrics.fPixelSize==22);xgeFontFree(fallback);
  }else{
   assert(xgeFontSaveXRF(font,argv[3],65,2)==XGE_OK);xge_font cached=NULL,loaded=NULL;
   assert(xgeFontLoadXRFOwned(&loaded,argv[3])==XGE_OK && xgeFontIsOwned(loaded));
   xgeFontFree(loaded);loaded=NULL;
   void* packed=NULL;int bytes=0;assert(xgeFontBuildXRFMemory(font,65,2,&packed,&bytes)==XGE_OK && bytes>0);
   assert(xgeFontLoadXRFMemoryOwned(&loaded,packed,bytes)==XGE_OK);xrtFree(packed);
   assert(xgeFontGetMetrics(loaded,&metrics)==XGE_OK && metrics.fPixelSize>0);xgeFontFree(loaded);
   assert(xgeFontLoadCachedOwned(&cached,argv[1],argv[4],20,65,2)==XGE_OK && xgeFontIsOwned(cached));
   xgeFontFree(cached);assert(!remove(argv[3]) && !remove(argv[4]));
  }
  xgeFontFree(font);assert(!xgeGraphicsOwnedResourceCount());
 }
#else
 else if(mode==2 || mode==6){assert(!"owned factories require the new matched SDK");}
#endif
 xgeFontFamilyFree(family);xgeFontFaceFree(regular);xgeFontFaceFree(bold);xrtFree(data);
 int unitStatus=xgeUnitChecked();
 if(unitStatus!=XGE_OK || xrtGetError())fprintf(stderr,"Final Unit=%d owners=%llu error=%s/%d: %s\n",unitStatus,(unsigned long long)xgeGraphicsOwnedResourceCount(),xrtGetError()?xrtErrorDomain(xrtGetError()):"none",xrtGetError()?xrtErrorCode(xrtGetError()):0,xrtGetError()?xrtErrorMessage(xrtGetError()):"none");
 assert(unitStatus==XGE_OK && !xrtGetError());xrtMemDebugSnapshot(&after);
 fprintf(stderr,"Final heap: before=%zu/%zu after=%zu/%zu invalid=%llu/%llu double=%llu/%llu\n",before.LiveCount,before.LiveBytes,after.LiveCount,after.LiveBytes,(unsigned long long)before.InvalidFreeCount,(unsigned long long)after.InvalidFreeCount,(unsigned long long)before.DoubleFreeCount,(unsigned long long)after.DoubleFreeCount);
 assert(after.LiveCount==before.LiveCount && after.LiveBytes==before.LiveBytes && after.InvalidFreeCount==before.InvalidFreeCount && after.DoubleFreeCount==before.DoubleFreeCount);
 printf("Font resources mode %u: native ownership/boundaries/memory balance passed\n",mode);return 0;
}
