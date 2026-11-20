#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include "xui.h"
#if !defined(XUI_TEST_STATIC)
#include <xge_host_xrt.h>
#include <xge_host_xrt_contract.h>
#endif
#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <string.h>

#if !defined(XUI_TEST_STATIC)
static xge_host_xrt_proc resolve(void* context,const char* name)
{
    (void)context;
#define XGE_HOST_XRT_SLOT(symbol) if(!strcmp(name,#symbol))return (xge_host_xrt_proc)&symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
    return NULL;
}
#endif

static int firstLegFailure(xui_code_document document)
{
    char text[2048];
    memset(text,'a',sizeof(text));
    assert(xuiCodeDocumentSetTextLength(document,text,sizeof(text))==XUI_OK);
    xui_code_range_t range={0};
    assert(xrtMemDebugFailAfter(0));
    int status=xuiCodeSearchFindPlain(document,"hi",1,XUI_CODE_SEARCH_WRAP,&range);
    assert(xrtMemDebugFailTriggered());
    xrtMemDebugFailClear();
    xrtClearError();
    if(status!=XUI_ERROR_OUT_OF_MEMORY){
        fprintf(stderr,"first-leg allocation failure returned %d instead of %d\n",status,XUI_ERROR_OUT_OF_MEMORY);
        return 1;
    }
    assert(xrtMemDebugFailAfter(0));
    status=xuiCodeSearchFindPlainRange(document,"hi",1,0,sizeof(text),XUI_CODE_SEARCH_WRAP,&range);
    assert(xrtMemDebugFailTriggered());
    xrtMemDebugFailClear();
    xrtClearError();
    assert(status==XUI_ERROR_OUT_OF_MEMORY);
    assert(range.iStart==0 && range.iEnd==0);
    return 0;
}

#if !defined(XUI_TEST_LEGACY_CONTROL)
static void typedQueries(xui_code_document document)
{
    const char text[]="A\0tag_12 tag";
    xui_code_range_t plain={0};
    xui_code_search_result_t regex={0};
    xui_find_result_t detached={0};
    char error[48];
    assert(xuiCodeDocumentSetTextLength(document,text,sizeof(text)-1)==XUI_OK);
    uint32_t version=xuiCodeDocumentGetChangeVersion(document);
    assert(xuiCodeSearchLocatePlain(document,"TAG",0,0,-1,0,&plain)==1);
    assert(plain.iStart==2 && plain.iEnd==5);
    assert(xuiCodeSearchLocatePlain(document,"tag",8,9,8,0,&plain)==0);
    assert(plain.iStart==0 && plain.iEnd==0);
    assert(xuiCodeSearchLocateRegex(document,"tag(_([0-9]+))?",7,7,-1,0,&regex,error,sizeof(error))==1);
    assert(!error[0] && regex.iStart==9 && regex.iEnd==12 && regex.iCaptureCount==3);
    assert(regex.arrCaptures[2].iStart==-1 && regex.arrCaptures[2].iEnd==-1);
    assert(xuiCodeSearchLocateRegex(document,"^tag",2,2,8,0,&regex,error,sizeof(error))==1);
    assert(regex.iStart==2 && regex.iEnd==5);
    assert(xuiCodeSearchLocateRegex(document,"missing",0,0,-1,0,&regex,error,sizeof(error))==0);
    assert(!error[0] && !regex.iSize);
    assert(xuiCodeSearchLocateRegex(document,"[",0,0,-1,0,&regex,error,sizeof(error))==XUI_ERROR_INVALID_ARGUMENT);
    assert(error[0]);xrtClearError();
    assert(xuiFindLocateText(text,sizeof(text)-1,"tag_([0-9]+)",0,0,-1,
        XUI_FIND_REGEX,&detached,error,sizeof(error))==1);
    assert(!error[0] && detached.iStart==2 && detached.arrCaptures[1].iStart==6);
    assert(xuiFindLocateText(NULL,0,"tag",0,0,-1,0,&detached,error,sizeof(error))==0);
    assert(!error[0] && !detached.iSize);
    assert(xuiFindLocateText("",0,"tag",0,0,-1,UINT32_MAX,&detached,error,sizeof(error))==XUI_ERROR_INVALID_ARGUMENT);
    assert(error[0]);xrtClearError();
    assert(xuiCodeDocumentGetChangeVersion(document)==version && !xuiCodeDocumentCanUndo(document));
}

static void hotAndBoundary(xui_code_document document)
{
    xui_code_range_t plain={0};
    assert(xuiCodeDocumentSetText(document,"abc abc")==XUI_OK);
    assert(xrtMemDebugFailAfter(0));
    for(int i=0;i<1000;++i){
        assert(xuiCodeSearchLocatePlain(document,"abc",0,0,-1,0,&plain)==1 && plain.iEnd==3);
        assert(xuiCodeSearchLocatePlain(document,"miss",0,0,-1,0,&plain)==0);
    }
    assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();
    const int length=2*65536+24;
    char* text=xrtMalloc((size_t)length);
    assert(text);
    memset(text,'a',(size_t)length);
    memcpy(text+65535,"xyz",3);
    assert(xuiCodeDocumentSetTextLength(document,text,length)==XUI_OK);
    xrtFree(text);
    assert(xuiCodeSearchLocatePlain(document,"xyz",0,0,-1,0,&plain)==1 && plain.iStart==65535);
    assert(xuiCodeSearchLocatePlain(document,"xyz",length,0,-1,XUI_CODE_SEARCH_BACKWARD,&plain)==1);
    assert(plain.iStart==65535 && plain.iEnd==65538);
}
#endif

int main(void)
{
#if !defined(XUI_TEST_STATIC)
    const xge_host_xrt_contract_t abi=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&abi,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
#endif
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    xui_code_document document=NULL;
    assert(xuiCodeDocumentCreate(&document)==XUI_OK);
    int status=firstLegFailure(document);
#if !defined(XUI_TEST_LEGACY_CONTROL)
    assert(!status);
    typedQueries(document);
    hotAndBoundary(document);
#endif
    assert(xuiCodeDocumentDestroyChecked(document)==XUI_OK);
    if(status==0)puts("XUI_CODE_SEARCH_NATIVE_OK");
    return status;
}
