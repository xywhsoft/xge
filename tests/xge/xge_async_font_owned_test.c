/* Generated valid font, synchronous caller-thread callbacks and owned transfer.
 * No system fonts, window, audio, network or malformed-font fuzzing. */
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
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#if !defined(XGE_TEST_STATIC)
static xge_host_xrt_proc resolve(void* unused,const char* name)
{
 (void)unused;
#define XGE_HOST_XRT_SLOT(symbol) if(!strcmp(name,#symbol))return (xge_host_xrt_proc)symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
 return NULL;
}
#endif
typedef struct Owner {
 unsigned refs,calls,action,failHold;
 uint64_t creator;
 xge_async_owned request;
 xge_font taken;
} Owner;
static bool count(const void* data,size_t* output){*output=((const Owner*)data)->refs;return true;}
static bool trace(const void* data,xrtownershipvisitor visit,void* user){(void)data;(void)visit;(void)user;return true;}
static const xrtownershipops ops={count,trace};
static bool hold(const void* data){Owner* o=(Owner*)data;if(o->failHold){--o->failHold;return false;}++o->refs;return true;}
static bool drop(const void* data){Owner* o=(Owner*)data;assert(o->refs>1);--o->refs;return true;}
static xge_callback_owner_t descriptor(Owner* o){return (xge_callback_owner_t){{o,&ops},hold,drop};}
static xge_async_load_info_t info(xge_async_owned request)
{xge_async_load_info_t v={0};v.size=sizeof(v);assert(xgeAsyncLoadInfoGet(request,&v)==XGE_OK);return v;}
static int callback(void* data)
{
 Owner* o=data;assert(xrtThreadCurrentId()==o->creator);++o->calls;
 assert(!info(o->request).threaded && info(o->request).status!=XGE_ASYNC_LOADING);
 if(o->action==4)assert(xgeAsyncLoadCallbackSetOwned(o->request,NULL,NULL,NULL)==XGE_OK);
 if(o->action==5){
  bool consumed=false;assert(xgeAsyncLoadReleaseOwnerChecked(o->request,&consumed)==XGE_OK && consumed);
  assert(info(o->request).hasResult);o->request=NULL;
 }
 if(o->action==6){
  assert(xgeAsyncLoadResetOwned(o->request)==XGE_ERROR_BUSY);
  assert(xgeAsyncFontStartOwned(o->request,"fontowned://input",20)==XGE_ERROR_BUSY);
  assert(xgeAsyncFontTakeOwned(o->request,&o->taken)==XGE_OK && xgeFontIsOwned(o->taken));
 }
 return o->action==7 ? -71:XGE_OK;
}
typedef struct Source {Owner* owner;unsigned char* data;int bytes;bool cancel;int fail;} Source;
static int provider(const char* path,void** data,int* bytes,void* user)
{
 (void)path;Source* s=user;assert(xrtThreadCurrentId()==s->owner->creator);
 if(s->cancel)assert(xgeAsyncLoadCancelOwned(s->owner->request)==XGE_OK);
 if(s->fail){xrtSetErrorKind(XERR_IO);return s->fail;}
 *data=s->data;*bytes=s->bytes;return XGE_OK;
}
static void borrowedFree(void* data,void* user){assert(data==((Source*)user)->data);}
static int32 wrongThread(void* data)
{
 xge_async_owned r=data;xge_font font=(xge_font)(uintptr_t)1;
 assert(xgeAsyncFontStartOwned(r,"fontowned://input",20)==XGE_ERROR_BUSY);
 assert(xgeAsyncFontTakeOwned(r,&font)==XGE_ERROR_BUSY && !font);
 bool consumed=true;assert(xgeAsyncLoadReleaseOwnerChecked(r,&consumed)==XGE_ERROR_BUSY && !consumed);
 return 0;
}
static unsigned visited;
static bool visit(xrtownershipref ref,void* expected){assert(ref.Data==expected && ref.Ops==&ops);++visited;return true;}
static void sameHeap(const xmemdebugsnapshot* a,const xmemdebugsnapshot* b)
{
 if(a->LiveCount!=b->LiveCount || a->LiveBytes!=b->LiveBytes)
  fprintf(stderr,"heap delta: %lld allocations, %lld bytes\n",(long long)b->LiveCount-(long long)a->LiveCount,(long long)b->LiveBytes-(long long)a->LiveBytes);
 assert(a->LiveCount==b->LiveCount && a->LiveBytes==b->LiveBytes);
 assert(a->InvalidFreeCount==b->InvalidFreeCount && a->DoubleFreeCount==b->DoubleFreeCount);
}
static void checkFont(xge_font font)
{
 assert(font && xgeFontIsOwned(font));xge_glyph_metrics_t glyph={0};
 assert(xgeFontGlyphGet(font,65,&glyph)==XGE_OK && glyph.fAdvanceX>0);
 xge_text_shape_desc_t d={0};d.iSize=sizeof(d);d.pFont=font;d.sText="ffi";d.iTextSize=3;d.fEmojiScale=1;
 xge_glyph_run_t* run=NULL;assert(xgeTextShapeOwned(&run,&d)==XGE_OK);
 const xge_glyph_position_t* values=NULL;size_t length=0;
 assert(xgeGlyphRunGetGlyphs(run,&values,&length)==XGE_OK && length>0);
 xgeGlyphRunFree(run);
}
int main(int argc,char** argv)
{
 assert(argc==3);int mode=atoi(argv[2]);assert(mode>=0 && mode<=15);
#if !defined(XGE_TEST_STATIC)
 const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_OK);
#endif
 assert(xrtMemDebugEnable(true));xge_desc_t engine={0};engine.iRunMode=XGE_RUN_MANUAL;assert(xgeInit(&engine)==XGE_OK);
 FILE* file=fopen(argv[1],"rb");assert(file && !fseek(file,0,SEEK_END));long length=ftell(file);assert(length>0 && length<INT_MAX);rewind(file);
 unsigned char* bytes=malloc((size_t)length);assert(bytes && fread(bytes,1,(size_t)length,file)==(size_t)length && !fclose(file));
 Owner owner={0};owner.refs=1;owner.creator=xrtThreadCurrentId();Source source={&owner,bytes,(int)length,false,0};
 xge_resource_provider_t p={"fontowned",provider,borrowedFree,&source};assert(xgeResourceProviderAdd(&p)==XGE_OK);
 xge_callback_owner_t own=descriptor(&owner);
 /* Warm ownership TLS and decoder before retained-allocation snapshots. */
 assert(xgeAsyncFontCreateOwned(&owner.request,&own)==XGE_OK);
 assert(xgeAsyncLoadCallbackSetOwned(owner.request,callback,&owner,&own)==XGE_OK);
 assert(xgeAsyncFontStartOwned(owner.request,"fontowned://input",20)==XGE_OK);
 xge_font warm=NULL;assert(xgeAsyncFontTakeOwned(owner.request,&warm)==XGE_OK);checkFont(warm);xgeFontFree(warm);
 assert(xgeAsyncLoadReleaseChecked(owner.request)==XGE_OK && owner.refs==1);owner.request=NULL;owner.calls=0;
 xrtClearError();xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
 assert(xgeAsyncFontCreateOwned(&owner.request,&own)==XGE_OK);xge_async_owned request=owner.request;
 if(mode==9){
  assert(xgeAsyncLoadReleaseChecked(request)==XGE_OK);request=owner.request=NULL;
  unsigned failures=0,successes=0;
  for(int budget=0;budget<64;++budget){
   xmemdebugsnapshot a,b;xrtMemDebugSnapshot(&a);assert(xrtMemDebugFailAfter(budget));
   int s=xgeAsyncFontCreateOwned(&request,&own);owner.request=request;
   if(s==XGE_OK)s=xgeAsyncFontStartOwned(request,"fontowned://input",20);
   if(s==XGE_OK)s=xgeAsyncLoadThrowIfFailed(request);
   xge_font font=NULL;if(s==XGE_OK)s=xgeAsyncFontTakeOwned(request,&font);
   if(s==XGE_OK)++successes;else ++failures;
   xrtMemDebugFailClear();xrtClearError();xgeFontFree(font);
   if(request)assert(xgeAsyncLoadReleaseChecked(request)==XGE_OK);
   request=owner.request=NULL;xrtClearError();xrtMemDebugSnapshot(&b);sameHeap(&a,&b);
   assert(owner.refs==1 && xgeGraphicsOwnedResourceCount()==0);
  }
  assert(failures && successes);printf("FontLoad allocation budgets: %u failures, %u successes\n",failures,successes);
 }else if(mode==8){
  xthread* t=xrtThreadCreate(wrongThread,request,0);assert(t && xrtThreadWait(t)==XWAIT_OK);xrtThreadDestroy(t);
 }else if(mode==10){
  const float invalid[]={NAN,INFINITY,-INFINITY,0,-1};
  for(unsigned i=0;i<sizeof(invalid)/sizeof(*invalid);++i){
   assert(xgeAsyncFontStartOwned(request,"fontowned://input",invalid[i])==XGE_OK);
   assert(info(request).status==XGE_ASYNC_FAILED && !info(request).hasResult && !xgeGraphicsOwnedResourceCount());
   assert(xgeAsyncLoadThrowIfFailed(request)==XGE_ERROR_INVALID_ARGUMENT);xrtClearError();
   assert(xgeAsyncLoadResetOwned(request)==XGE_OK);
  }
  assert(xgeAsyncFontStartOwned(request,"missing-owned-font.ttf",20)==XGE_OK);
  assert(info(request).status==XGE_ASYNC_FAILED && xgeAsyncLoadThrowIfFailed(request)!=XGE_OK);xrtClearError();
 }else if(mode==12){
  assert(xgeAsyncLoadCallbackSetOwned(request,callback,&owner,&own)==XGE_OK);
  xrtownershipref ref=xgeAsyncLoadOwnership(request);size_t refs=0;assert(ref.Ops->Count(ref.Data,&refs) && refs==1);
  visited=0;assert(ref.Ops->Trace(ref.Data,visit,&owner) && visited==2);
 }else if(mode==14){
  /* Empty requests retain the existing union's 248-byte total allocation. */
  xrtMemDebugSnapshot(&after);assert(after.LiveCount-before.LiveCount==1 && after.LiveBytes-before.LiveBytes==248);
  printf("Empty FontLoad: %llu allocations / %llu bytes\n",(unsigned long long)(after.LiveCount-before.LiveCount),(unsigned long long)(after.LiveBytes-before.LiveBytes));
  assert(xgeAsyncLoadReleaseChecked(request)==XGE_OK);request=owner.request=NULL;
  assert(xgeAsyncImageCreateOwned(&request,&own)==XGE_OK);xrtMemDebugSnapshot(&after);
  assert(after.LiveCount-before.LiveCount==1 && after.LiveBytes-before.LiveBytes==248);
  assert(xgeAsyncLoadReleaseChecked(request)==XGE_OK);request=NULL;
  assert(xgeAsyncTextureCreateOwned(&request,&own)==XGE_OK);xrtMemDebugSnapshot(&after);
  assert(after.LiveCount-before.LiveCount==1 && after.LiveBytes-before.LiveBytes==248);
  puts("Existing ImageLoad/TextureLoad retained allocation size unchanged");
 }else if(mode==15){
  for(int budget=0;budget<2;++budget){
   assert(xrtMemDebugFailAfter(budget));assert(xgeAsyncFontStartOwned(request,"fontowned://input",20)==XGE_ERROR_OUT_OF_MEMORY);
   xrtMemDebugFailClear();xrtClearError();assert(info(request).status==XGE_ASYNC_PENDING && !info(request).hasResult);
   assert(!xgeGraphicsOwnedResourceCount());
  }
  assert(xgeAsyncFontStartOwned(request,"fontowned://input",20)==XGE_OK && info(request).hasResult);
 }else{
  owner.action=(unsigned)mode;source.cancel=mode==3;source.fail=mode==13 ? XGE_ERROR_CALLBACK_FAILED:0;
  assert(xgeAsyncThreadingSet(mode==1 || mode==2)==XGE_OK);
  assert(xgeAsyncLoadCallbackSetOwned(request,callback,&owner,&own)==XGE_OK);
  int s=xgeAsyncFontStartOwned(request,mode==1 ? argv[1]:"fontowned://input",20);
  assert(s==(mode==7 ? -71:mode==13 ? XGE_ERROR_CALLBACK_FAILED:XGE_OK));
  if(mode==5){request=NULL;assert(owner.refs==1 && !xgeGraphicsOwnedResourceCount());}
  else if(mode==3 || mode==13){
   assert(info(request).status==(mode==3 ? XGE_ASYNC_CANCELLED:XGE_ASYNC_FAILED) && !info(request).hasResult && owner.calls==0);
   assert(!xgeGraphicsOwnedResourceCount() && xgeAsyncLoadThrowIfFailed(request)!=XGE_OK);xrtClearError();
  }else{
   assert(info(request).status==XGE_ASYNC_READY && !info(request).threaded && owner.calls==1);
   assert(xgeGraphicsOwnedResourceCount()==1 && xgeUnitChecked()==XGE_ERROR_BUSY);xrtClearError();
   xge_font font=owner.taken;owner.taken=NULL;
   if(!font){
    assert(xrtMemDebugFailAfter(0));assert(xgeAsyncFontTakeOwned(request,&font)==XGE_OK && !xrtMemDebugFailTriggered());xrtMemDebugFailClear();
   }
   assert(xgeGraphicsOwnedResourceCount()==1 && !info(request).hasResult);
   xge_font duplicate=(xge_font)(uintptr_t)1;assert(xgeAsyncFontTakeOwned(request,&duplicate)==XGE_ERROR_INVALID_STATE && !duplicate);
   if(mode==11){
    assert(xgeAsyncLoadResetOwned(request)==XGE_OK && info(request).status==XGE_ASYNC_PENDING);
    assert(xgeAsyncFontStartOwned(request,argv[1],18)==XGE_OK && info(request).hasResult && xgeGraphicsOwnedResourceCount()==2);
    assert(xgeAsyncLoadResetOwned(request)==XGE_OK && xgeGraphicsOwnedResourceCount()==1);
   }
   xge_async_owned shared=NULL;assert(xgeAsyncLoadShareOwned(request,&shared)==XGE_OK);
   assert(xgeAsyncLoadReleaseChecked(request)==XGE_OK);request=owner.request=NULL;
   assert(xgeAsyncLoadReleaseChecked(shared)==XGE_OK && owner.refs==1);
   checkFont(font);xgeFontFree(font);assert(!xgeGraphicsOwnedResourceCount());
  }
 }
 if(request)assert(xgeAsyncLoadReleaseChecked(request)==XGE_OK);
 assert(owner.refs==1 && !xgeGraphicsOwnedResourceCount());xrtClearError();xrtMemDebugSnapshot(&after);sameHeap(&before,&after);
 assert(xgeResourceProviderClearChecked()==XGE_OK && xgeUnitChecked()==XGE_OK);free(bytes);
 printf("FontLoad mode %d: ownership/callback/cleanup passed\n",mode);return 0;
}
