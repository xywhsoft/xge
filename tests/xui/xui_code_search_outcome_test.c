/* Real SDK search/replace: counted document text and first cleanup outcome. */
#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include <xge.h>
#include <xui.h>
#if defined(XGE_TEST_STATIC)
#include <xge_static_xrt_contract.h>
#else
#include <xge_host_xrt_contract.h>
static xge_host_xrt_proc resolve(void* user,const char* name)
{
 (void)user;
#define XGE_HOST_XRT_SLOT(symbol) if(!strcmp(name,#symbol))return (xge_host_xrt_proc)&symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
 return NULL;
}
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct Owner {size_t refs;unsigned calls,gone;bool fail,close;xerror* error;} Owner;
static bool count(const void* raw,size_t* out){*out=((const Owner*)raw)->refs;return true;}
static bool trace(const void* raw,xrtownershipvisitor visit,void* user){(void)raw;(void)visit;(void)user;return true;}
static const xrtownershipops ops={count,trace};
static bool hold(const void* raw){++((Owner*)raw)->refs;return true;}
static bool drop(const void* raw)
{
 Owner* owner=(Owner*)raw;assert(owner->refs>1);--owner->refs;
 if(owner->error)xrtSetErrorTake(xrtErrorRef(owner->error));
 return !owner->fail;
}
static void gone(void* raw){++((Owner*)raw)->gone;}
static void changed(xui_code_document document,xui_code_range_t range,uint32_t version,void* raw)
{
 Owner* owner=raw;(void)range;(void)version;++owner->calls;
 if(owner->close){bool consumed=false;assert(xuiCodeDocumentDestroyOwnerChecked(document,&consumed)==XUI_OK&&consumed&&!owner->gone);}
}
static bool equals(xui_code_document document,const char* text,size_t length)
{xstrview value={0};assert(xuiCodeDocumentGetTextView(document,&value)==XUI_OK);return value.Size==length&&!memcmp(value.Data,text,length);}
static int replace(xui_code_document document,int regex,const char* pattern,const char* replacement,int* number)
{char error[128]={0};return regex ? xuiCodeSearchReplaceAllRegex(document,pattern,replacement,0,number,error,sizeof(error)):
 xuiCodeSearchReplaceAllPlain(document,pattern,replacement,0,number);}
static int nul(void)
{
 const char before[]="a\0foo foo",after[]="a\0bar bar";int number=-1;
 xui_code_document document=NULL;assert(xuiCodeDocumentCreate(&document)==XUI_OK);
 assert(xuiCodeDocumentSetTextLength(document,before,sizeof(before)-1)==XUI_OK);
 int status=replace(document,1,"foo","bar",&number);
 bool ok=status==XUI_OK&&number==2&&equals(document,after,sizeof(after)-1);
 if(!ok)printf("Regex replacement truncated counted document: status=%d count=%d length=%d expected=%zu\n",status,number,xuiCodeDocumentGetLength(document),sizeof(after)-1);
 if(ok){assert(xuiCodeDocumentUndo(document)==XUI_OK&&equals(document,before,sizeof(before)-1));
  assert(xuiCodeDocumentRedo(document)==XUI_OK&&equals(document,after,sizeof(after)-1));}
 assert(xuiCodeDocumentDestroyChecked(document)==XUI_OK);return ok ? 0:83;
}
static int newlines(void)
{
 xui_code_document document=NULL;int number=-1;assert(xuiCodeDocumentCreate(&document)==XUI_OK);
 assert(xuiCodeDocumentSetText(document,"xx")==XUI_OK);
 int status=replace(document,0,"x","\r\n",&number);
 bool ok=status==XUI_OK&&number==2&&equals(document,"\n\n",2);
 if(!ok)printf("Plain replacement skipped after CRLF normalization: status=%d count=%d\n",status,number);
 if(ok){assert(xuiCodeDocumentUndo(document)==XUI_OK&&equals(document,"xx",2));
  assert(xuiCodeDocumentRedo(document)==XUI_OK&&equals(document,"\n\n",2));
  assert(xuiCodeDocumentSetText(document,"xx")==XUI_OK);
  assert(replace(document,0,"x","x",&number)==XUI_OK&&number==2&&equals(document,"xx",2));}
 assert(xuiCodeDocumentDestroyChecked(document)==XUI_OK);return ok ? 0:84;
}
static int first_failure(void)
{
 unsigned partial=0,wrong=0;
 for(int allowance=0;allowance<48;++allowance){
  xui_code_document document=NULL;assert(xuiCodeDocumentCreate(&document)==XUI_OK);
  assert(xuiCodeDocumentSetText(document,"foo foo foo")==XUI_OK);
  Owner owner={.refs=1,.fail=true};owner.error=xrtErrorCreate(XERR_STATE,"replace-cleanup",70,"later cleanup error");assert(owner.error);
  xui_callback_owner_t binding={{&owner,&ops},hold,drop};
  assert(xuiCodeDocumentSetChangeCallbackOwned(document,changed,&owner,&binding)==XUI_OK);
  assert(xrtMemDebugFailAfter(allowance));int number=-1,status=replace(document,0,"foo","bar",&number);xrtMemDebugFailClear();
  if(status==XUI_ERROR_OUT_OF_MEMORY){if(owner.calls)++partial;if(xrtGetError()==owner.error)++wrong;}
  else assert(status==XUI_ERROR_CALLBACK_FAILED&&owner.calls==1&&xrtGetError()==owner.error);
  xrtClearError();owner.fail=false;xrtErrorFree(owner.error);owner.error=NULL;
  assert(xuiCodeDocumentSetChangeCallbackOwned(document,NULL,NULL,NULL)==XUI_OK&&owner.refs==1);
  assert(xuiCodeDocumentDestroyChecked(document)==XUI_OK);
 }
 assert(partial>0);
 if(wrong)printf("Earlier search failure replaced by later EndEdit diagnostic: %u prefixes\n",wrong);
 return wrong ? 85:0;
}
static void callback_outcomes(int mode)
{
 for(int regex=0;regex<2;++regex){
  xui_code_document document=NULL;assert(xuiCodeDocumentCreate(&document)==XUI_OK);
  assert(xuiCodeDocumentSetText(document,"foo foo")==XUI_OK);
  Owner owner={.refs=1,.fail=mode<5,.close=mode==5};
  if(mode==4){owner.error=xrtErrorCreate(XERR_STATE,"replace-owner",71,"callback failure");assert(owner.error);}
  xui_callback_owner_t binding={{&owner,&ops},hold,drop};xui_lifetime_link_t watch={0};
  assert(xuiCodeDocumentSetChangeCallbackOwned(document,changed,&owner,&binding)==XUI_OK);
  assert(xuiCodeDocumentLifetimeWatch(document,&watch,gone,&owner)==XUI_OK);
  int number=-1,status=replace(document,regex,"foo","bar",&number);
  assert(status==(mode==5 ? XUI_ERROR_INVALID_STATE:XUI_ERROR_CALLBACK_FAILED)&&owner.calls==1);
  assert(xrtGetError()==owner.error);xrtClearError();owner.fail=false;owner.close=false;xrtErrorFree(owner.error);owner.error=NULL;
  if(mode==5)assert(owner.refs==1&&owner.gone==1&&!watch.ppPrevious);
  else{assert(equals(document,"bar bar",7));assert(xuiCodeDocumentSetChangeCallbackOwned(document,NULL,NULL,NULL)==XUI_OK);
   assert(xuiCodeDocumentDestroyChecked(document)==XUI_OK&&owner.gone==1&&owner.refs==1);}
 }
}
static void regex_allocations(void)
{
 unsigned failed=0,succeeded=0;
 for(int allowance=0;allowance<96;++allowance){
  xui_code_document document=NULL;assert(xuiCodeDocumentCreate(&document)==XUI_OK);
  assert(xuiCodeDocumentSetText(document,"foo foo")==XUI_OK);
  assert(xrtMemDebugFailAfter(allowance));int number=-1,status=replace(document,1,"(foo)","$1!",&number);xrtMemDebugFailClear();
  assert(status==XUI_OK||status==XUI_ERROR_OUT_OF_MEMORY);xrtClearError();
  if(status==XUI_OK){assert(number==2&&equals(document,"foo! foo!",9));++succeeded;}
  else{assert(equals(document,"foo foo",7));++failed;}
  assert(xuiCodeDocumentDestroyChecked(document)==XUI_OK);
 }
 assert(failed&&succeeded);
}
static int partial_grapheme(void)
{
 const char before[]="e\xcc\x81",after[]="X\xcc\x81";xui_code_document document=NULL;int number=-1;
 assert(xuiCodeDocumentCreate(&document)==XUI_OK&&xuiCodeDocumentSetTextLength(document,before,sizeof(before)-1)==XUI_OK);
 int status=replace(document,0,"e","X",&number);
 bool ok=status==XUI_OK&&number==1&&equals(document,after,sizeof(after)-1);
 if(!ok)printf("Plain replacement expanded partial grapheme: status=%d count=%d length=%d expected=%zu\n",status,number,xuiCodeDocumentGetLength(document),sizeof(after)-1);
 if(ok){assert(xuiCodeDocumentUndo(document)==XUI_OK&&equals(document,before,sizeof(before)-1));
  assert(xuiCodeDocumentRedo(document)==XUI_OK&&equals(document,after,sizeof(after)-1));}
 assert(xuiCodeDocumentDestroyChecked(document)==XUI_OK);return ok ? 0:86;
}
int main(int argc,char** argv)
{
 assert(argc==2);int mode=atoi(argv[1]);assert(mode>=0&&mode<=7);
#if defined(XGE_TEST_STATIC)
 const xge_host_xrt_contract_t contract=XGE_STATIC_XRT_LOCAL_CONTRACT_INIT;
 assert(!strcmp(contract.headerSha256,xgeStaticXrtContractV1.headerSha256));
#else
 const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
 assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
#endif
 assert(xrtMemDebugEnabled()||xrtMemDebugEnable(true));int status=0;
 if(mode==0)status=nul();else if(mode==1)status=newlines();else if(mode==2)status=first_failure();
 else if(mode==6)regex_allocations();else if(mode==7)status=partial_grapheme();else callback_outcomes(mode);
 xrtClearError();xmemdebugsnapshot end;xrtMemDebugSnapshot(&end);assert(!end.LiveCount&&!end.InvalidFreeCount&&!end.DoubleFreeCount);
 if(!status)printf("Code search outcome mode %d passed\n",mode);
 return status;
}
