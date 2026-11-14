/* Actual SDK clients; counted instrumentation is test-only. */
#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include "xui.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned compiles,matchers,releases,copies;
static size_t copiedBytes;
#if defined(XGE_TEST_STATIC)
static bool pretendLimit;
#include <xge_static_xrt_contract.h>
extern xregex* __real_xrtRegexCompileConfig(xstrview,const xregexconfig*);
extern xregexmatcher* __real_xrtRegexMatcherCreate(xregex*);
extern void __real_xrtRegexRelease(xregex*);
extern int __real_xuiCodeDocumentGetLength(xui_code_document);
extern int __real_xuiCodeDocumentCopyRange(xui_code_document,int,int,char*,int,int*);
xregex* __wrap_xrtRegexCompileConfig(xstrview text,const xregexconfig* config)
{++compiles;return __real_xrtRegexCompileConfig(text,config);}
xregexmatcher* __wrap_xrtRegexMatcherCreate(xregex* regex)
{++matchers;return __real_xrtRegexMatcherCreate(regex);}
void __wrap_xrtRegexRelease(xregex* regex)
{++releases;__real_xrtRegexRelease(regex);}
int __wrap_xuiCodeDocumentGetLength(xui_code_document document)
{return pretendLimit ? INT_MAX:__real_xuiCodeDocumentGetLength(document);}
int __wrap_xuiCodeDocumentCopyRange(xui_code_document document,int start,int end,char* text,int capacity,int* length)
{++copies;copiedBytes+=(size_t)(end-start);return __real_xuiCodeDocumentCopyRange(document,start,end,text,capacity,length);}
#else
#include <xge_host_xrt_contract.h>
static xregex* compile_counted(xstrview text,const xregexconfig* config)
{++compiles;return xrtRegexCompileConfig(text,config);}
static xregexmatcher* matcher_counted(xregex* regex)
{++matchers;return xrtRegexMatcherCreate(regex);}
static void release_counted(xregex* regex)
{++releases;xrtRegexRelease(regex);}
static xge_host_xrt_proc resolve(void* user,const char* name)
{
 (void)user;
 if(!strcmp(name,"xrtRegexCompileConfig"))return (xge_host_xrt_proc)&compile_counted;
 if(!strcmp(name,"xrtRegexMatcherCreate"))return (xge_host_xrt_proc)&matcher_counted;
 if(!strcmp(name,"xrtRegexRelease"))return (xge_host_xrt_proc)&release_counted;
#define XGE_HOST_XRT_SLOT(symbol) if(!strcmp(name,#symbol))return (xge_host_xrt_proc)&symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
 return NULL;
}
#endif
static void reset(void){compiles=matchers=releases=copies=0;copiedBytes=0;}
static int mark(xui_code_document document,xui_code_annotation_store store,const char* pattern,uint32_t flags,int* count)
{char error[80];return xuiCodeSearchMarkAllRegex(document,store,pattern,flags,123,count,error,sizeof(error));}
static void indicator(xui_code_annotation_store store,int position,int count,int start,int end)
{
 xui_code_indicator_t out[3];int number=-1;
 assert(xuiCodeAnnotationGetIndicatorsAt(store,position,out,3,&number)==XUI_OK&&number==count);
 if(count)assert(out[0].iIndicator==123&&out[0].tRange.iStart==start&&out[0].tRange.iEnd==end);
}
static void semantics(xui_code_document document,xui_code_annotation_store store)
{
 int number=-1;const char text[]="a\0Foo foo food\nfoo_1 foo_2\n";
 assert(xuiCodeDocumentSetTextLength(document,text,sizeof(text)-1)==XUI_OK);
 assert(mark(document,store,"foo",XUI_CODE_SEARCH_WHOLE_WORD,&number)==XUI_OK&&number==2);
 indicator(store,2,1,2,5);indicator(store,6,1,6,9);indicator(store,10,0,0,0);
 assert(mark(document,store,"foo_[0-9]",XUI_CODE_SEARCH_CASE_SENSITIVE|XUI_CODE_SEARCH_BACKWARD|XUI_CODE_SEARCH_WRAP,&number)==XUI_OK&&number==2);
 indicator(store,15,1,15,20);indicator(store,21,1,21,26);
 assert(mark(document,store,"^",XUI_CODE_SEARCH_MULTILINE,&number)==XUI_OK&&number==0);
 assert(mark(document,store,"[",0,&number)==XUI_ERROR_INVALID_ARGUMENT&&number==0);
 xrtClearError();
 assert(mark(document,store,"missing",0,&number)==XUI_OK&&number==0);
 assert(xuiCodeDocumentSetText(document,"你a\néa")==XUI_OK);
 assert(mark(document,store,"a|^",XUI_CODE_SEARCH_MULTILINE,&number)==XUI_OK&&number==2);
 indicator(store,3,1,3,4);indicator(store,8,1,8,9);
}
static int performance(xui_code_document document,xui_code_annotation_store store)
{
 char text[257];for(int i=0;i<64;++i)memcpy(text+4*i,"foo ",4);text[256]=0;
 assert(xuiCodeDocumentSetText(document,text)==XUI_OK);int count=-1;
 assert(mark(document,store,"foo",0,&count)==XUI_OK&&count==64);
 xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);reset();
 int status=mark(document,store,"foo",0,&count);xrtMemDebugSnapshot(&after);
 printf("MarkAll cost count=%d compiles=%u matchers=%u releases=%u copies=%u copiedBytes=%zu alloc=%llu free=%llu realloc=%llu\n",count,compiles,matchers,releases,copies,copiedBytes,
  (unsigned long long)(after.AllocCount-before.AllocCount),(unsigned long long)(after.FreeCount-before.FreeCount),(unsigned long long)(after.ReallocCount-before.ReallocCount));
 assert(status==XUI_OK&&count==64);
 bool once=compiles==1&&matchers==1&&releases==1;
#if defined(XGE_TEST_STATIC)
 once=once&&copies==1&&copiedBytes==256;
#endif
 if(!once)puts("MarkAll repeats regex compilation and whole-document copy");
 return once ? 0:88;
}
static int limit(xui_code_document document)
{
#if defined(XGE_TEST_STATIC)
 assert(xuiCodeDocumentSetText(document,"foo")==XUI_OK);reset();pretendLimit=true;
 xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);int number=37;
 int status=xuiCodeSearchReplaceAllRegex(document,"foo","bar",0,&number,NULL,0);
 pretendLimit=false;xrtClearError();xrtMemDebugSnapshot(&after);
 assert(status==XUI_ERROR_LIMIT_EXCEEDED&&number==0&&compiles==1);
 if(releases!=1||after.LiveCount!=before.LiveCount||after.LiveBytes!=before.LiveBytes){
  puts("ReplaceAll length-limit branch leaked compiled regex");return 87;
 }
#else
 (void)document;assert(!"limit injection is static-only");
#endif
 return 0;
}
static void allocation_prefixes(xui_code_document document)
{
 unsigned failed=0,succeeded=0;assert(xuiCodeDocumentSetText(document,"foo foo foo")==XUI_OK);
 for(int allowance=0;allowance<512;++allowance){
  xui_code_annotation_store store=NULL;assert(xuiCodeAnnotationStoreCreate(&store)==XUI_OK);
  assert(xrtMemDebugFailAfter(allowance));int number=37,status=mark(document,store,"(foo)",0,&number);xrtMemDebugFailClear();
  assert(status==XUI_OK||status==XUI_ERROR_OUT_OF_MEMORY);xrtClearError();
  if(status==XUI_OK){assert(number==3);++succeeded;}else ++failed;
  xuiCodeAnnotationStoreDestroy(store);
 }
 assert(failed&&succeeded);
}
int main(int argc,char** argv)
{
 assert(argc==2);int mode=atoi(argv[1]);assert(mode>=0&&mode<=3);
#if defined(XGE_TEST_STATIC)
 const xge_host_xrt_contract_t contract=XGE_STATIC_XRT_LOCAL_CONTRACT_INIT;
 assert(!strcmp(contract.headerSha256,xgeStaticXrtContractV1.headerSha256));
#else
 const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
 assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
#endif
 assert(xrtMemDebugEnable(true));xui_code_document document=NULL;xui_code_annotation_store store=NULL;
 assert(xuiCodeDocumentCreate(&document)==XUI_OK&&xuiCodeAnnotationStoreCreate(&store)==XUI_OK);
 int status=0;
 if(mode==0)semantics(document,store);else if(mode==1)status=performance(document,store);else if(mode==2)status=limit(document);else allocation_prefixes(document);
 xuiCodeAnnotationStoreDestroy(store);assert(xuiCodeDocumentDestroyChecked(document)==XUI_OK);xrtClearError();
 xmemdebugsnapshot end;xrtMemDebugSnapshot(&end);if(status!=87)assert(!end.LiveCount&&!end.InvalidFreeCount&&!end.DoubleFreeCount);
 if(!status)printf("MarkAll actual SDK mode %d passed\n",mode);
 return status;
}
