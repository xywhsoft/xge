/* CPU-only column text ownership, legacy borrowing, failure and reentry. */
#include "xui_internal.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int fail_next_malloc, failed_allocations, track_next_malloc;
static void* column_allocations[64];
static int column_allocation_count, live_column_allocations;
void* __real_xrtMalloc(size_t size);
void* __wrap_xrtMalloc(size_t size)
{
    if (fail_next_malloc) { fail_next_malloc=0; ++failed_allocations; return NULL; }
    void* result=__real_xrtMalloc(size);
    if (track_next_malloc && result) {
        assert(column_allocation_count<64);
        column_allocations[column_allocation_count++]=result;
        ++live_column_allocations;track_next_malloc=0;
    }
    return result;
}

void __real_xrtFree(void* pointer);
void __wrap_xrtFree(void* pointer)
{
    for (int i=0;i<column_allocation_count;++i) if (pointer && column_allocations[i]==pointer) {
        column_allocations[i]=NULL;--live_column_allocations;break;
    }
    __real_xrtFree(pointer);
}
static int set_copied_columns(xui_widget table,const xui_table_view_column_t* columns,int count)
{
    track_next_malloc=count>0;
    int result=xuiTableViewSetColumnsCopy(table,columns,count);
    track_next_malloc=0;
    return result;
}

typedef struct fixture_t { int armed; int calls; int nested; } fixture_t;
static int count_rows(xui_widget table, void* user)
{
    fixture_t* state=user;
    ++state->calls;
    if (state->armed) {
        state->armed=0;
        xui_table_view_column_t nested={0};nested.sTitle="nested";nested.fWidth=120;
        state->nested=set_copied_columns(table,&nested,1);
    }
    return 2;
}
int main(void)
{
    xui_proxy proxy=NULL;xui_context context=NULL;xui_widget table=NULL,grid=NULL;
    assert(xuiProxyXgeCreate(&proxy)==XUI_OK);
    assert(xuiCreateOwned(proxy,&context)==XUI_OK);
    assert(xuiTableViewCreate(context,&table,NULL)==XUI_OK);
    char id[]="first",title[]="Original";
    xui_table_view_column_t column={0};column.sId=id;column.sTitle=title;column.fWidth=140;
    assert(set_copied_columns(table,&column,1)==XUI_OK);
    memset(id,'x',sizeof(id)-1);memset(title,'x',sizeof(title)-1);
    const xui_table_view_column_t* stored=xuiTableViewGetColumn(table,0);
    assert(stored && !strcmp(stored->sId,"first") && !strcmp(stored->sTitle,"Original"));
    xui_table_view_column_t alias=*stored;alias.fWidth=150;
    assert(xuiTableViewSetColumns(table,&alias,1)==XUI_OK);
    assert(!strcmp(xuiTableViewGetColumn(table,0)->sTitle,"Original"));
    assert(set_copied_columns(table,&column,-1)==XUI_ERROR_INVALID_ARGUMENT);
    assert(!strcmp(xuiTableViewGetColumn(table,0)->sTitle,"Original"));
    column.sTitle="replacement";fail_next_malloc=1;
    assert(set_copied_columns(table,&column,1)==XUI_ERROR_OUT_OF_MEMORY);
    assert(failed_allocations==1 && !fail_next_malloc);
    assert(!strcmp(xuiTableViewGetColumn(table,0)->sTitle,"Original"));
    puts("column allocation failure preserved previous state");
    char borrowed[]="borrowed";column.sTitle=borrowed;column.sId=NULL;
    assert(xuiTableViewSetColumns(table,&column,1)==XUI_OK);
    borrowed[0]='B';assert(!strcmp(xuiTableViewGetColumn(table,0)->sTitle,"Borrowed"));
    fixture_t fixture={0};
    assert(xuiTableViewSetAdapter(table,count_rows,NULL,&fixture)==XUI_OK);
    fixture.armed=1;column.sTitle="outer";
    int status=set_copied_columns(table,&column,1);
    assert(status==XUI_OK || status==XUI_ERROR_INVALID_STATE);
    assert(!fixture.armed && fixture.calls>0);
    assert(fixture.nested==XUI_OK || fixture.nested==XUI_ERROR_INVALID_STATE);
    stored=xuiTableViewGetColumn(table,0);
    assert(stored && (!strcmp(stored->sTitle,"outer") || !strcmp(stored->sTitle,"nested") || !strcmp(stored->sTitle,"Borrowed")));
    assert(set_copied_columns(table,NULL,0)==XUI_OK);
    assert(xuiTableViewGetColumnCount(table)==0);
    assert(xuiTableGridCreate(context,&grid,NULL)==XUI_OK);
    column.sTitle="grid";
    track_next_malloc=1;
    assert(xuiTableGridSetColumnsCopy(grid,&column,1)==XUI_OK);
    track_next_malloc=0;
    assert(xuiDestroyChecked(context)==XUI_OK);
    assert(column_allocation_count>=4 && live_column_allocations==0);
    printf("column storage allocations released: %d\n",column_allocation_count);
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK);
    return 0;
}
