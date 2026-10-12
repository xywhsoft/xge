/* Exercise the demo's public toolbar, actual find window and native renderer.
 * Synthetic pointer/key events do not constitute physical IME acceptance. */
#define main document_demo_original_main
#include "../examples/xui/xui_document/main.c"
#undef main

static demo app;
static const char* destination;
static const char* mode_name;
static unsigned checks;
static int with_format;
#define REQUIRE(e) do { checks++; if (!(e)) { fprintf(stderr,"find line %d: %s\n",__LINE__,#e); \
    app.result=XUI_ERROR_INVALID_STATE; xgeQuit(); return app.result; } } while(0)

static int ui_click(xui_widget widget)
{
    xui_rect_t rect; int result;
    if (!widget) return XUI_ERROR_INVALID_ARGUMENT;
    result=xuiLayout(app.context); if(result!=XUI_OK)return result;
    rect=xuiWidgetGetWorldRect(widget);
    if(rect.fW<=0 || rect.fH<=0)return XUI_ERROR_INVALID_STATE;
    result=xuiInputPointerMove(app.context,rect.fX+rect.fW/2,rect.fY+rect.fH/2,0);
    if(result==XUI_OK)result=xuiInputPointerDown(app.context,rect.fX+rect.fW/2,rect.fY+rect.fH/2,XUI_POINTER_BUTTON_LEFT,XUI_POINTER_BUTTON_LEFT);
    if(result==XUI_OK)result=xuiInputPointerUp(app.context,rect.fX+rect.fW/2,rect.fY+rect.fH/2,XUI_POINTER_BUTTON_LEFT,0);
    if(result==XUI_OK)result=xuiDispatchPendingEvents(app.context);
    if(result==XUI_OK)result=xuiUpdate(app.context,.016f);
    return result;
}
static int ui_key(int key,uint32_t modifiers)
{
    int result=xuiInputKeyDown(app.context,key,modifiers);
    if(result==XUI_OK)result=xuiInputKeyUp(app.context,key,0);
    if(result==XUI_OK)result=xuiDispatchPendingEvents(app.context);
    if(result==XUI_OK)result=xuiUpdate(app.context,.016f);
    return result;
}
static xui_widget child(xui_widget parent,xui_widget_type type,int ordinal)
{
    xui_widget widget;
    for(widget=xuiWidgetGetFirstChild(parent);widget;widget=xuiWidgetGetNextSibling(widget))
        if(xuiWidgetIsType(widget,type) && ordinal--==0)return widget;
    return NULL;
}
static xui_widget toolbar(const char* name)
{
    xui_widget widget;
    for(widget=xuiWidgetGetFirstChild(app.root);widget;widget=xuiWidgetGetNextSibling(widget)) {
        const char* text=xuiButtonGetText(widget);
        if(text && !strcmp(text,name))return widget;
    }
    return NULL;
}
static xui_widget control(xui_widget window,int text_id,int checkbox)
{
    xui_widget widget; const char* wanted=xuiTranslate(app.context,text_id);
    for(widget=xuiWidgetGetFirstChild(xuiWindowGetClientWidget(window));widget;widget=xuiWidgetGetNextSibling(widget)) {
        const char* text=checkbox?xuiCheckBoxGetText(widget):xuiButtonGetText(widget);
        if(text && !strcmp(text,wanted))return widget;
    }
    return NULL;
}
static int capture(const char* name)
{
    char path[1024]; xui_rect_i_t damage={0,0,W,H}; uint64_t a,b; int result;
    result=xuiLayout(app.context);
    if(result==XUI_OK)result=xuiUpdate(app.context,.016f);
    if(result==XUI_OK)result=app.proxy.surfaceClear(&app.proxy,app.target,XUI_COLOR_WHITE);
    if(result==XUI_OK)result=xuiRender(app.context,app.target,&damage,1);
    snprintf(path,sizeof(path),"%s/%s",destination,name);
    if(result==XUI_OK)result=capture_async(&app,path,&a,&b);
    return result;
}
static int content_is(xui_document document,const char* expected)
{
    xui_document_snapshot snapshot=NULL; char* plain=NULL; uint64_t bytes; int result;
    result=xuiDocumentAcquireSnapshot(document,&snapshot);
    if(result==XUI_OK)result=xuiDocumentSnapshotCopyPlainText(snapshot,&plain,&bytes);
    if(result==XUI_OK && strcmp(plain,expected)) {
        fprintf(stderr,"Expected [%s], got [%s]\n",expected,plain);result=XUI_ERROR_INVALID_STATE;
    }
    xuiDocumentFreeBuffer(plain);xuiDocumentSnapshotRelease(snapshot);return result;
}
static int query(xui_widget editor,xui_widget input,xui_widget window,const char* text,uint64_t expected)
{
    uint64_t count=UINT64_MAX;
    REQUIRE(xuiInputSetText(input,text)==XUI_OK);
    REQUIRE(ui_click(control(window,XUI_TR_FIND_ALL,0))==XUI_OK);
    REQUIRE(xuiDocumentViewGetFindResultCount(editor,&count)==XUI_OK && count==expected);
    REQUIRE(xuiTableViewGetRowCount(child(xuiWindowGetClientWidget(window),xuiTableViewGetType(app.context),0))==(int)expected);
    return XUI_OK;
}
static int run_find(void)
{
    xui_widget editor=!strcmp(mode_name,"rich")?app.rich:app.markdown;
    xui_widget window,input,replacement,status,case_box,word_box,regex_box;
    xui_document document=NULL; xui_doc_desc_t desc={0}; xui_doc_range_t range;
    uint32_t mode=!strcmp(mode_name,"source")?XUI_DOC_SOURCE_TEXT:
        !strcmp(mode_name,"live")?XUI_DOC_LIVE_MARKDOWN:XUI_DOC_VISUAL;
    uint64_t revision,count; int active=0;
    desc.iSize=sizeof(desc);desc.iProfile=editor==app.rich?XUI_DOCUMENT_RICH:XUI_DOCUMENT_MARKDOWN;
    REQUIRE(xuiDocumentCreate(&desc,&document)==XUI_OK);
    REQUIRE(replace_document(&app,editor,document)==XUI_OK);xuiDocumentRelease(document);
    document=xuiDocumentViewGetDocument(editor);
    if(editor==app.rich)REQUIRE(xuiDocumentEditorInsertText(editor,"alpha beta alpha",16)==XUI_OK);
    else REQUIRE(xuiDocumentLoadMarkdown(document,"alpha beta alpha\n",17)==XUI_OK);
    REQUIRE(xuiDocumentViewSetMode(editor,mode)==XUI_OK);
    REQUIRE(xuiSetFocusWidget(app.context,editor)==XUI_OK);app.active_editor=editor;
    revision=xuiDocumentGetRevision(document);
    REQUIRE(ui_click(toolbar("Find"))==XUI_OK);
    window=xuiDocumentEditorGetFindWindow(editor);
    REQUIRE(window && xuiWindowIsOpen(window));
    input=child(xuiWindowGetClientWidget(window),xuiInputGetType(app.context),0);
    replacement=child(xuiWindowGetClientWidget(window),xuiInputGetType(app.context),1);
    status=child(xuiWindowGetClientWidget(window),xuiLabelGetType(app.context),0);
    case_box=control(window,XUI_TR_FIND_CASE,1);word_box=control(window,XUI_TR_FIND_WORD,1);regex_box=control(window,XUI_TR_FIND_REGEX,1);
    REQUIRE(input && replacement && status && case_box && word_box && regex_box);
    REQUIRE(xuiGetFocusWidget(app.context)==input);
    REQUIRE(query(editor,input,window,"alpha",2)==XUI_OK);
    REQUIRE(ui_click(control(window,XUI_TR_FIND_NEXT,0))==XUI_OK);
    REQUIRE(xuiDocumentViewGetSelection(editor,&range)==XUI_OK);
    REQUIRE(range.tAnchor.iNodeId!=0 || range.tAnchor.iKind==XUI_DOC_POSITION_SOURCE);
    REQUIRE(ui_click(control(window,XUI_TR_FIND_PREVIOUS,0))==XUI_OK);
    REQUIRE(capture("find.png")==XUI_OK);
    REQUIRE(query(editor,input,window,"ALPHA",0)==XUI_OK);
    REQUIRE(ui_click(case_box)==XUI_OK);
    REQUIRE(query(editor,input,window,"ALPHA",2)==XUI_OK);
    REQUIRE(query(editor,input,window,"alp",2)==XUI_OK);
    REQUIRE(ui_click(word_box)==XUI_OK);
    REQUIRE(query(editor,input,window,"alp",0)==XUI_OK);
    REQUIRE(ui_click(word_box)==XUI_OK);
    REQUIRE(ui_click(regex_box)==XUI_OK);
    REQUIRE(query(editor,input,window,"al.ha",2)==XUI_OK);
    REQUIRE(query(editor,input,window,"[",0)==XUI_OK);
    REQUIRE(!strcmp(xuiLabelGetText(status),xuiTranslate(app.context,XUI_TR_FIND_INVALID_PATTERN)));
    REQUIRE(xuiDocumentGetRevision(document)==revision);
    REQUIRE(content_is(document,"alpha beta alpha\n")==XUI_OK);
    REQUIRE(capture("invalid-pattern.png")==XUI_OK);
    REQUIRE(ui_click(regex_box)==XUI_OK);
    REQUIRE(query(editor,input,window,"alpha",2)==XUI_OK);
    REQUIRE(ui_click(toolbar("Replace"))==XUI_OK);
    REQUIRE(xuiDocumentEditorGetFindWindow(editor)==window && xuiWindowIsOpen(window));
    REQUIRE(xuiWidgetGetVisible(replacement));
    REQUIRE(xuiInputSetText(replacement,"Z")==XUI_OK);
    REQUIRE(ui_click(control(window,XUI_TR_FIND_NEXT,0))==XUI_OK);
    REQUIRE(ui_click(control(window,XUI_TR_REPLACE_CURRENT,0))==XUI_OK);
    REQUIRE(xuiDocumentViewGetFindResultCount(editor,&count)==XUI_OK && count==1);
    REQUIRE(capture("replace-current.png")==XUI_OK);
    REQUIRE(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_UNDO)==XUI_OK);
    REQUIRE(content_is(document,"alpha beta alpha\n")==XUI_OK);
    REQUIRE(ui_click(control(window,XUI_TR_REPLACE_ALL,0))==XUI_OK);
    REQUIRE(content_is(document,"Z beta Z\n")==XUI_OK);
    REQUIRE(xuiDocumentViewGetFindResultCount(editor,&count)==XUI_OK && count==0);
    REQUIRE(capture("replace-all.png")==XUI_OK);
    REQUIRE(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_UNDO)==XUI_OK);
    REQUIRE(content_is(document,"alpha beta alpha\n")==XUI_OK);
    REQUIRE(xuiDocumentViewGetFindResultCount(editor,&count)==XUI_OK && count==2);
    REQUIRE(xuiDocumentViewGetFindResult(editor,0,&range,&active)==XUI_OK);
    REQUIRE(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_REDO)==XUI_OK);
    REQUIRE(content_is(document,"Z beta Z\n")==XUI_OK);
    REQUIRE(xuiSetFocusWidget(app.context,input)==XUI_OK);
    REQUIRE(ui_key(XUI_KEY_ESCAPE,0)==XUI_OK);
    REQUIRE(!xuiWindowIsOpen(window) && xuiGetFocusWidget(app.context)==editor);
    REQUIRE(ui_key('F',XUI_MOD_CTRL)==XUI_OK);
    REQUIRE(xuiWindowIsOpen(window) && xuiGetFocusWidget(app.context)==input);
    REQUIRE(ui_key('H',XUI_MOD_CTRL)==XUI_OK && xuiWidgetGetVisible(replacement));
    REQUIRE(ui_key(XUI_KEY_ESCAPE,0)==XUI_OK);
    REQUIRE(xuiGetFocusWidget(app.context)==editor);
    REQUIRE(xuiInputText(app.context,0x4e2d)==XUI_OK && xuiDispatchPendingEvents(app.context)==XUI_OK);
    REQUIRE(xuiDocumentEditorFlush(editor)==XUI_OK);
    REQUIRE(xuiDocumentViewGetDocument(editor)==document);
    if(editor==app.markdown)REQUIRE(xuiDocumentViewGetDocument(app.preview)==document);
    REQUIRE(capture("resume-edit.png")==XUI_OK);
    return XUI_OK;
}
static int serialized_document(xui_document document,char** bytes,uint64_t* length)
{
    xui_document_snapshot snapshot=NULL;int result=xuiDocumentAcquireSnapshot(document,&snapshot);
    if(result==XUI_OK)result=xuiDocumentSerialize(snapshot,bytes,length);
    xuiDocumentSnapshotRelease(snapshot);return result;
}
static int same_document(xui_document document,const char* expected,uint64_t length)
{
    char* actual=NULL;uint64_t bytes=0;int result=serialized_document(document,&actual,&bytes);
    int same=result==XUI_OK && bytes==length && !memcmp(actual,expected,(size_t)bytes);
    xuiDocumentFreeBuffer(actual);return same;
}
static int menu_pointer(xui_widget menu,int index)
{
    xui_rect_t world=xuiWidgetGetWorldRect(menu),row=xuiMenuGetItemRect(menu,index);
    float x=world.fX+row.fX+row.fW/2,y=world.fY+row.fY+row.fH/2;int result;
    result=xuiInputPointerMove(app.context,x,y,0);
    if(result==XUI_OK)result=xuiInputPointerDown(app.context,x,y,XUI_POINTER_BUTTON_LEFT,XUI_POINTER_BUTTON_LEFT);
    if(result==XUI_OK)result=xuiInputPointerUp(app.context,x,y,XUI_POINTER_BUTTON_LEFT,0);
    if(result==XUI_OK)result=xuiDispatchPendingEvents(app.context);
    if(result==XUI_OK)result=xuiUpdate(app.context,.016f);
    return result;
}
static int run_format(void)
{
    const uint32_t commands[]={XUI_DOC_EDIT_PARAGRAPH,XUI_DOC_EDIT_HEADING_1,XUI_DOC_EDIT_HEADING_2,
        XUI_DOC_EDIT_HEADING_3,XUI_DOC_EDIT_BULLET_LIST,XUI_DOC_EDIT_NUMBER_LIST,XUI_DOC_EDIT_TASK_LIST,
        XUI_DOC_EDIT_BLOCK_QUOTE,XUI_DOC_EDIT_INSERT_CODE_BLOCK,XUI_DOC_EDIT_UNDERLINE,
        XUI_DOC_EDIT_STRIKE,XUI_DOC_EDIT_CODE,XUI_DOC_EDIT_CLEAR_FORMATTING};
    xui_widget editor=!strcmp(mode_name,"rich")?app.rich:app.markdown;
    xui_widget button=toolbar("Format"),menu;uint32_t mode=!strcmp(mode_name,"source")?XUI_DOC_SOURCE_TEXT:
        !strcmp(mode_name,"live")?XUI_DOC_LIVE_MARKDOWN:XUI_DOC_VISUAL;
    size_t command;int capture_menu=1;unsigned initial=checks;
    REQUIRE(button);
    for(command=0;command<sizeof(commands)/sizeof(commands[0]);command++) {
        xui_doc_desc_t desc={0};xui_document document=NULL;xui_doc_command_state_t state={0};
        char *before=NULL,*after=NULL;uint64_t before_bytes,after_bytes,revision;int index,selected=-1,enabled;
        desc.iSize=sizeof(desc);desc.iProfile=editor==app.rich?XUI_DOCUMENT_RICH:XUI_DOCUMENT_MARKDOWN;
        REQUIRE(xuiDocumentCreate(&desc,&document)==XUI_OK);
        REQUIRE(replace_document(&app,editor,document)==XUI_OK);xuiDocumentRelease(document);
        document=xuiDocumentViewGetDocument(editor);
        if(editor==app.rich)REQUIRE(xuiDocumentEditorInsertText(editor,"alpha beta",10)==XUI_OK);
        else REQUIRE(xuiDocumentLoadMarkdown(document,"alpha beta\n",11)==XUI_OK);
        REQUIRE(xuiDocumentViewSetMode(editor,mode)==XUI_OK);
        REQUIRE(xuiSetFocusWidget(app.context,editor)==XUI_OK);app.active_editor=editor;
        REQUIRE(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_SELECT_ALL)==XUI_OK);
        if(commands[command]==XUI_DOC_EDIT_INSERT_CODE_BLOCK) {
            xui_doc_range_t caret;
            REQUIRE(xuiDocumentViewGetSelection(editor,&caret)==XUI_OK);caret.tAnchor=caret.tCaret;
            REQUIRE(xuiDocumentViewSetSelection(editor,&caret)==XUI_OK);
        }
        if(commands[command]==XUI_DOC_EDIT_CLEAR_FORMATTING && mode==XUI_DOC_VISUAL)
            REQUIRE(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_BOLD)==XUI_OK);
        REQUIRE(serialized_document(document,&before,&before_bytes)==XUI_OK);
        revision=xuiDocumentGetRevision(document);
        REQUIRE(ui_click(button)==XUI_OK);
        menu=(xui_widget)xuiWidgetGetUserData(button);REQUIRE(menu && xuiMenuIsOpen(menu));
        REQUIRE(xuiMenuGetOwner(menu)==editor);
        for(index=0;index<xuiMenuGetItemCount(menu);index++) {
            const xui_menu_item_t* item=xuiMenuGetItem(menu,index);
            if(item->iType==XUI_MENU_ITEM_SEPARATOR)continue;
            state.iSize=sizeof(state);REQUIRE(xuiDocumentEditorQueryCommand(editor,(uint32_t)item->iValue,&state)==XUI_OK);
            REQUIRE(!!(item->iState&XUI_MENU_ITEM_ENABLED)==!!state.bEnabled);
            REQUIRE(!!(item->iState&XUI_MENU_ITEM_CHECKED)==!!state.bActive);
            if(item->iValue==(int)commands[command])selected=index;
        }
        REQUIRE(selected>=0);enabled=!!(xuiMenuGetItemState(menu,selected)&XUI_MENU_ITEM_ENABLED);
        if(commands[command]==XUI_DOC_EDIT_INSERT_CODE_BLOCK && mode==XUI_DOC_VISUAL)REQUIRE(enabled);
        if(capture_menu) {REQUIRE(capture("format-menu.png")==XUI_OK);capture_menu=0;}
        if(enabled && commands[command]==XUI_DOC_EDIT_HEADING_1) {
            REQUIRE(xuiMenuSetHoverIndex(menu,selected)==XUI_OK);
            REQUIRE(ui_key(XUI_KEY_ENTER,0)==XUI_OK);
        } else REQUIRE(menu_pointer(menu,selected)==XUI_OK);
        if(!enabled) {
            REQUIRE(xuiMenuIsOpen(menu));REQUIRE(xuiDocumentGetRevision(document)==revision && same_document(document,before,before_bytes));
            REQUIRE(ui_key(XUI_KEY_ESCAPE,0)==XUI_OK);
        } else {
            REQUIRE(!xuiMenuIsOpen(menu));REQUIRE(xuiDocumentEditorFlush(editor)==XUI_OK);
            REQUIRE(serialized_document(document,&after,&after_bytes)==XUI_OK);
            if(commands[command]!=XUI_DOC_EDIT_PARAGRAPH) {
                REQUIRE(xuiDocumentGetRevision(document)>revision);
                REQUIRE(!same_document(document,before,before_bytes));
                REQUIRE(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_UNDO)==XUI_OK && same_document(document,before,before_bytes));
                REQUIRE(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_REDO)==XUI_OK && same_document(document,after,after_bytes));
            }
        }
        REQUIRE(xuiGetFocusWidget(app.context)==editor);
        REQUIRE(xuiDocumentViewGetDocument(editor)==document);
        if(editor==app.markdown)REQUIRE(xuiDocumentViewGetDocument(app.preview)==document);
        if(commands[command]==XUI_DOC_EDIT_HEADING_1 && enabled)REQUIRE(capture("format-result.png")==XUI_OK);
        xuiDocumentFreeBuffer(before);xuiDocumentFreeBuffer(after);
    }
    REQUIRE(xuiDocumentEditorSetReadOnly(editor,1)==XUI_OK);
    REQUIRE(ui_click(button)==XUI_OK);menu=(xui_widget)xuiWidgetGetUserData(button);
    for(int index=0;index<xuiMenuGetItemCount(menu);index++)REQUIRE(!(xuiMenuGetItemState(menu,index)&XUI_MENU_ITEM_ENABLED));
    REQUIRE(ui_key(XUI_KEY_ESCAPE,0)==XUI_OK && xuiGetFocusWidget(app.context)==editor);
    REQUIRE(xuiDocumentEditorSetReadOnly(editor,0)==XUI_OK);
    {
        const int sizes[][2]={{640,760},{960,640},{1440,900},{W,H}};
        uint64_t revision=xuiDocumentGetRevision(xuiDocumentViewGetDocument(editor));
        for(size_t size=0;size<sizeof(sizes)/sizeof(sizes[0]);size++) {
            xui_rect_t rect;
            REQUIRE(arrange(&app,sizes[size][0],sizes[size][1])==XUI_OK);
            REQUIRE(ui_click(button)==XUI_OK && xuiMenuIsOpen(menu));
            rect=xuiWidgetGetWorldRect(menu);
            REQUIRE(rect.fX>=0 && rect.fY>=0 && rect.fW>0 && rect.fH>0 &&
                rect.fX+rect.fW<=sizes[size][0] && rect.fY+rect.fH<=sizes[size][1]);
            REQUIRE(xuiMenuGetOwner(menu)==editor);
            REQUIRE(ui_key(XUI_KEY_ESCAPE,0)==XUI_OK && xuiGetFocusWidget(app.context)==editor);
            REQUIRE(xuiDocumentGetRevision(xuiDocumentViewGetDocument(editor))==revision);
        }
    }
    if(mode!=XUI_DOC_VISUAL)REQUIRE(capture("format-result.png")==XUI_OK);
    printf("Native Document format %s: 13 commands, %u checks passed.\n",mode_name,checks-initial);
    return XUI_OK;
}
static int find_frame(void* unused)
{
    int result;(void)unused;
    if(!app.context && (result=resources(&app))!=XUI_OK) {app.result=result;xgeQuit();return XGE_OK;}
    result=run_find();if(result==XUI_OK && with_format)result=run_format();
    if(result!=XUI_OK)app.result=result;
    xgeQuit();return XGE_OK;
}
int main(int argc,char** argv)
{
    xge_desc_t window={0};int result;
    if(argc!=3 && argc!=4)return 2;
    with_format=argc==4 && !strcmp(argv[3],"format");
    destination=argv[1];mode_name=argv[2];
    if(strcmp(mode_name,"rich") && strcmp(mode_name,"source") && strcmp(mode_name,"visual") && strcmp(mode_name,"live"))return 2;
    window.iWidth=W;window.iHeight=H;window.sTitle="Document find/replace workflows";
    window.iRunMode=XGE_RUN_GAME_LOOP;window.iFlags=XGE_INIT_VSYNC;
    if(xgeInit(&window)!=XGE_OK)return 2;
    result=xgeRun(find_frame,NULL);if(result==XGE_OK)result=app.result;
    xgeSetQuitRequestCallback(NULL,NULL);
    xuiDestroy(app.context);xuiDocumentRelease(app.rich_doc);xuiDocumentRelease(app.md_doc);
    if(app.target)app.proxy.surfaceDestroy(&app.proxy,app.target);
    if(app.proxy.fontDestroy) {app.proxy.fontDestroy(&app.proxy,app.mono);app.proxy.fontDestroy(&app.proxy,app.italic);
        app.proxy.fontDestroy(&app.proxy,app.bold);app.proxy.fontDestroy(&app.proxy,app.font);}
    xgeFontFallbackClear();xgeUnit();
    if(result==XUI_OK)printf("Native Document find %s: %u checks passed.\n",mode_name,checks);
    return result==XUI_OK?0:1;
}
