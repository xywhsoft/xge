#include "xui.h"
#include "xui_internal.h"
#undef NDEBUG
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
static void gone(void* user){++*(int*)user;}
static void same_live(xmemdebugsnapshot a,xmemdebugsnapshot b)
{assert(a.LiveCount==b.LiveCount && a.LiveBytes==b.LiveBytes && a.InvalidFreeCount==b.InvalidFreeCount && a.DoubleFreeCount==b.DoubleFreeCount);}
int main(void)
{
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    xui_context context=NULL;assert(xuiCreate(&context)==XUI_OK);
    xui_language english=xuiGetLanguagePack(context,XUI_LANGUAGE_EN);
    assert(english && xuiLanguageGetContext(english)==context);
    assert(xuiLanguageGetTextCount(english)==XUI_TR_COUNT-1);
    assert(xuiLanguageGetText(english,XUI_TR_NONE)==NULL);
    assert(xuiLanguageGetText(english,XUI_TR_COUNT)==NULL);
    assert(xuiLanguageGetText(NULL,1)==NULL && xuiLanguageGetTextCount(NULL)==0);
    bool removed=true;
    assert(xuiLanguageRollbackCreate(english,&removed)==XUI_ERROR_INVALID_STATE && !removed);
    xui_language custom=xuiCreateLanguage(context,"test","Test language",XUI_LANGUAGE_EN);
    assert(custom && xuiLanguageGetText(custom,XUI_TR_EDIT_COPY)==NULL);
    int id=xuiGetLanguageId(custom);assert(id>=XUI_LANGUAGE_CUSTOM_BASE);
    assert(!strcmp(xuiGetLanguageCode(custom),"test") && !strcmp(xuiGetLanguageName(custom),"Test language"));
    uint32_t revision=xuiGetLanguageRevision(context);
    assert(xuiLanguageSetText(context,custom,XUI_TR_EDIT_COPY,"")==XUI_OK);
    assert(xuiLanguageGetText(custom,XUI_TR_EDIT_COPY) && !*xuiLanguageGetText(custom,XUI_TR_EDIT_COPY));
    assert(xuiGetLanguageRevision(context)!=revision);
    assert(xuiLanguageSetText(context,custom,XUI_TR_EDIT_COPY,"Ready")==XUI_OK);
    assert(xuiSetLanguage(context,id)==XUI_OK && !strcmp(xuiTranslate(context,XUI_TR_EDIT_COPY),"Ready"));
    assert(xuiLanguageRollbackCreate(custom,&removed)==XUI_ERROR_INVALID_STATE && !removed);
    assert(xuiSetLanguage(context,XUI_LANGUAGE_EN)==XUI_OK);
    xui_language dependent=xuiCreateLanguage(context,"dependent","Dependent",id);
    assert(dependent);
    assert(xuiLanguageRollbackCreate(custom,&removed)==XUI_ERROR_BUSY && !removed);
    assert(xuiLanguageRollbackCreate(dependent,&removed)==XUI_OK && removed);
    int invalidated=0;xui_lifetime_link_t watch={0};
    assert(xuiLanguageLifetimeWatch(custom,&watch,gone,&invalidated)==XUI_OK);
    assert(xuiLanguageRollbackCreate(custom,&removed)==XUI_OK && removed && invalidated==1);
    assert(xuiGetLanguagePack(context,id)==NULL);xuiLifetimeUnwatch(&watch);
    xui_language output=(xui_language)(uintptr_t)1;
    assert(xuiCreateLanguageOwned(NULL,"x","X",0,&output)==XUI_ERROR_INVALID_ARGUMENT && output==NULL);
    assert(xuiCreateLanguageOwned(context,NULL,"X",0,&output)==XUI_ERROR_INVALID_ARGUMENT && output==NULL);
    assert(xuiCreateLanguageOwned(context,"x","X",0,NULL)==XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiCreateLanguageOwned(context,"owned","Owned",0,&output)==XUI_OK && output);
    assert(xuiLanguageRollbackCreate(output,&removed)==XUI_OK && removed);
    // Publication failure releases every partial native allocation.
    int failed=0,succeeded=0;
    for(int allowance=0;allowance<48;++allowance){
        xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
        assert(xrtMemDebugFailAfter(allowance));
        xui_language candidate=xuiCreateLanguage(context,"candidate","Candidate",XUI_LANGUAGE_EN);
        xrtMemDebugFailClear();
        if(candidate){++succeeded;assert(xuiLanguageRollbackCreate(candidate,&removed)==XUI_OK && removed);}
        else ++failed;
        xrtMemDebugSnapshot(&after);same_live(before,after);
    }
    assert(failed && succeeded);
    // Replace failure preserves the old slot and language revision.
    revision=xuiGetLanguageRevision(context);const char* original=xuiLanguageGetText(english,XUI_TR_EDIT_COPY);
    assert(xrtMemDebugFailAfter(0));assert(xuiLanguageSetText(context,english,XUI_TR_EDIT_COPY,"changed")==XUI_ERROR_OUT_OF_MEMORY);xrtMemDebugFailClear();
    assert(xuiLanguageGetText(english,XUI_TR_EDIT_COPY)==original && xuiGetLanguageRevision(context)==revision);
    context->iNextCustomLanguageId=INT_MAX;
    assert(xuiCreateLanguage(context,"exhausted","Exhausted",XUI_LANGUAGE_EN)==NULL && context->iNextCustomLanguageId==INT_MAX);
    assert(xuiCreateLanguageOwned(context,"exhausted","Exhausted",0,&output)==XUI_ERROR_INVALID_STATE && output==NULL);
    assert(xuiLanguageLifetimeWatch(english,&watch,gone,&invalidated)==XUI_OK);
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    for(int i=0;i<1000;++i){assert(xuiLanguageGetTextCount(english)==XUI_TR_COUNT-1);assert(xuiLanguageGetText(english,XUI_TR_EDIT_COPY)==original);}
    xrtMemDebugSnapshot(&after);assert(before.AllocCount==after.AllocCount && before.FreeCount==after.FreeCount && before.ReallocCount==after.ReallocCount);
    xuiDestroy(context);assert(invalidated==2);xuiLifetimeUnwatch(&watch);
    puts("Localization: Context-owned packs, watched rollback, NULL/empty, 48 native allocation budgets and bounded IDs passed");
    return 0;
}
