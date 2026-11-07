#include "xui.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <string.h>

#define CHECK(e) do { ++checks; if(!(e)){fprintf(stderr,"code_find_session:%d: %s\n",__LINE__,#e);failed=1;goto cleanup;} } while(0)
static xui_widget child(xui_context context,xui_widget w,const char* type,const char* label,int* index)
{
    if(!w)return NULL;
    if(xuiWidgetGetType(w)==xuiWidgetFindType(context,type)) {
        const char* text=!strcmp(type,"button") ? xuiButtonGetText(w):!strcmp(type,"checkbox") ? xuiCheckBoxGetText(w):"";
        if((!label || !strcmp(text,label)) && (*index)--==0)return w;
    }
    for(xui_widget c=xuiWidgetGetFirstChild(w);c;c=xuiWidgetGetNextSibling(c)){xui_widget found=child(context,c,type,label,index);if(found)return found;}
    return NULL;
}
static xui_widget control(xui_context context,xui_widget window,const char* type,int label,int index)
{return child(context,window,type,label ? xuiTranslate(context,label):NULL,&index);}
static int editor(xui_context context,xui_widget root,const char* text,xui_widget* out)
{
    xui_code_edit_desc_t desc={0};desc.iSize=sizeof(desc);desc.sText=text;desc.sLanguage="c";
    int result=xuiCodeEditCreate(context,out,&desc);
    if(result==XUI_OK)result=xuiWidgetAddChild(root,*out);
    return result;
}
static void destroyScope(xui_code_find_scope scope,xui_widget editor,const xui_code_find_result_t* result,void* user)
{(void)editor;(void)result;*(int*)user=0;xuiCodeFindScopeDestroy(scope);*(int*)user=result->iEnd>result->iStart;}
int main(void)
{
    int failed=0,checks=0,destroyed=0;
    xui_context context=NULL,other=NULL;xui_code_find_scope scope=NULL;
    xui_widget root=NULL,a=NULL,b=NULL,c=NULL; xui_font font=NULL;xui_test_proxy_state_t state;
    xuiTestProxyInit(&state);
    CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&state.tProxy)==XUI_OK);
    CHECK(state.tProxy.fontLoadFile(&state.tProxy,&font,"find.ttf",14,0)==XUI_OK && xuiSetDefaultFont(context,font)==XUI_OK);
    CHECK(xuiInputViewport(context,800,600)==XUI_OK && xuiWidgetCreate(context,&root)==XUI_OK && xuiSetRootWidget(context,root)==XUI_OK);
    CHECK(xuiWidgetSetRect(root,(xui_rect_t){0,0,800,600})==XUI_OK && editor(context,root,"alpha\n",&a)==XUI_OK && editor(context,root,"alpha alpha\n",&b)==XUI_OK);
    CHECK(xuiCodeFindScopeCreate(&scope)==XUI_OK && xuiCodeFindScopeAddEditorNamed(scope,a,"one.c")==XUI_OK && xuiCodeFindScopeAddEditorNamed(scope,b,"two.c")==XUI_OK);
    CHECK(xuiCodeEditOpenReplace(a)==XUI_OK);
    xui_widget window=xuiCodeFindScopeGetFindWindow(scope);
    CHECK(window && window==xuiCodeEditGetFindWindow(a) && window==xuiCodeEditGetFindWindow(b));
    xui_widget query=control(context,window,"input",0,0),replacement=control(context,window,"input",0,1),all=control(context,window,"checkbox",XUI_TR_FIND_SCOPE,0);
    CHECK(query && replacement && all && xuiInputSetText(query,"alpha")==XUI_OK && xuiCodeFindScopeRefresh(scope)==XUI_OK && xuiCodeEditGetFindResultCount(a)==1);
    CHECK(xuiSetFocusWidget(context,b)==XUI_OK && xuiCodeFindScopeSetActiveEditor(scope,b)==XUI_OK && xuiGetFocusWidget(context)==b && xuiCodeFindScopeRefresh(scope)==XUI_OK && xuiCodeEditGetFindResultCount(b)==2);
    CHECK(xuiCodeFindScopeGetActiveEditor(scope)==b && !strcmp(xuiInputGetText(query),"alpha") && xuiCodeFindScopeGetFindWindow(scope)==window);
    CHECK(xuiCheckBoxSetChecked(all,1)==XUI_OK && xuiCodeFindScopeRefresh(scope)==XUI_OK && xuiCodeFindScopeGetResultCount(scope)==3);
    CHECK(xuiCodeFindScopeSetActiveEditor(scope,NULL)==XUI_OK && xuiCodeFindScopeFindNext(scope,0)==XUI_OK && xuiCodeFindScopeGetActiveEditor(scope)==a);
    CHECK(xuiCodeFindScopeFindNext(scope,0)==XUI_OK && xuiCodeFindScopeGetActiveEditor(scope)==b);
    CHECK(xuiInputSetText(replacement,"changed")==XUI_OK && xuiCodeFindScopeSetActiveEditor(scope,b)==XUI_OK && xuiDispatchPendingEvents(context)==XUI_OK && xuiUpdate(context,.1f)==XUI_OK && xuiLayout(context)==XUI_OK);
    xui_widget replaceAll=control(context,window,"button",XUI_TR_REPLACE_ALL,0);xui_rect_t rect=xuiWidgetGetWorldRect(replaceAll);float x=rect.fX+rect.fW/2,y=rect.fY+rect.fH/2;
    CHECK(replaceAll && rect.fW>0 && rect.fH>0 && xuiInputPointerDown(context,x,y,XUI_POINTER_BUTTON_LEFT,XUI_POINTER_BUTTON_LEFT)==XUI_OK && xuiInputPointerUp(context,x,y,XUI_POINTER_BUTTON_LEFT,0)==XUI_OK && xuiDispatchPendingEvents(context)==XUI_OK);
    CHECK(!strcmp(xuiCodeEditGetText(a),"alpha\n") && !strcmp(xuiCodeEditGetText(b),"changed changed\n"));
    CHECK(xuiCodeDocumentUndo(xuiCodeEditGetDocument(b))==XUI_OK && !strcmp(xuiCodeEditGetText(b),"alpha alpha\n"));
    CHECK(xuiCodeFindScopeRefresh(scope)==XUI_OK && xuiCodeFindScopeGetResultCount(scope)==3);
    CHECK(xuiCodeDocumentInsert(xuiCodeEditGetDocument(a),0,"prefix ")==XUI_OK && xuiCodeFindScopeGetResultCount(scope)==0 && xuiCodeFindScopeActivateResult(scope,0)!=XUI_OK);
    CHECK(xuiWidgetDestroyChecked(a)==XUI_OK);a=NULL;
    CHECK(xuiCodeFindScopeGetEditorCount(scope)==1 && xuiCodeFindScopeGetFindWindow(scope)==window && xuiWindowIsOpen(window));
    CHECK(xuiCodeFindScopeRefresh(scope)==XUI_OK && xuiCodeFindScopeGetResultCount(scope)==2);
    CHECK(xuiCodeFindScopeSetActivate(scope,destroyScope,&destroyed)==XUI_OK && xuiCodeFindScopeFindNext(scope,0)==XUI_OK && destroyed);scope=NULL;
    CHECK(xuiCodeEditGetFindScope(b)==NULL && xuiCodeEditOpenFind(b)==XUI_OK && xuiCodeEditGetFindWindow(b)!=NULL);
    CHECK(xuiWidgetDestroyChecked(b)==XUI_OK);b=NULL;
    CHECK(editor(context,root,"last\n",&c)==XUI_OK && xuiCodeFindScopeCreate(&scope)==XUI_OK && xuiCodeFindScopeAddEditor(scope,c)==XUI_OK && xuiCodeEditOpenFind(c)==XUI_OK);
    CHECK(xuiCodeFindScopeSetActiveEditor(scope,root)==XUI_ERROR_INVALID_ARGUMENT);
    xuiDestroy(context);context=NULL;c=NULL;root=NULL;
    CHECK(xuiCodeFindScopeGetEditorCount(scope)==0 && xuiCodeFindScopeGetFindWindow(scope)==NULL);
    xuiCodeFindScopeDestroy(scope);scope=NULL;
    CHECK(xuiCreate(&other)==XUI_OK);xuiDestroy(other);other=NULL;
cleanup:
    if(scope)xuiCodeFindScopeDestroy(scope);
    if(context)xuiDestroy(context);
    if(other)xuiDestroy(other);
    if(font)state.tProxy.fontDestroy(&state.tProxy,font);
    if(!failed)printf("code_find_session passed: %d native checks; shared identity, target/focus, scoped navigation, current replacement/undo, stale offsets, editor-first/context-first cleanup and reentrant scope destruction\n",checks);
    return failed;
}
