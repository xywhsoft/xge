#define XRT_MODULE_ALL
#include "xui_document.h"
#include "extlibs/xrt/xrt.h"
#include "src/xui/document/xui_document_internal.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const xui_doc_txn_desc_t edit={.iSize=sizeof(edit),.iDomain=XUI_DOC_SEMANTIC};
static void* (*allocate_document)(void*,size_t);
static int allocation_budget;

static void* fail_document_alloc(void* user,size_t bytes)
{
    if(allocation_budget--==0)return NULL;
    return allocate_document(user,bytes);
}

static int check_write_failure(xui_document document,doc_allocator* allocator,
    uint64_t parent,uint64_t node,const xui_doc_node_attributes_view_t* attrs,
    const xui_doc_node_desc_t* insert)
{
    uint64_t baseline=atomic_load(&allocator->live);
    allocate_document=allocator->alloc;
    for(int budget=0;budget<64;++budget){
        xui_document_transaction txn=NULL;
        assert(xuiDocumentBeginTransaction(document,&edit,&txn)==XUI_OK);
        uint64_t output=0;
        allocation_budget=budget;
        allocator->alloc=fail_document_alloc;
        int result=insert
            ? xuiDocumentTxnInsertNodeWithAttributes(txn,parent,XUI_DOCUMENT_APPEND,attrs,insert,&output)
            : xuiDocumentTxnSetNodeAttributes(txn,node,attrs);
        allocator->alloc=allocate_document;
        xuiDocumentTxnAbort(txn);
        assert(xuiDocumentTxnReleaseChecked(txn)==XUI_OK);
        assert(atomic_load(&allocator->live)==baseline);
        if(result==XUI_OK){
            assert(!insert || output);
            return budget;
        }
        assert(result==XUI_ERROR_OUT_OF_MEMORY && !output);
    }
    return -1;
}

static void check_style(const xui_doc_node_attributes_view_t* result,size_t size)
{
    assert(result->tValues.iMarks==XUI_DOC_BOLD && result->tValues.fFontSize==18);
    assert(result->tValues.iTextColor==0xEE4422FF && result->tValues.iRowSpan==1);
    assert(result->iFontFamilyBytes==size && result->sFontFamily[size]==0);
    assert(result->sLanguage && !strcmp(result->sLanguage,"zh-hans"));
}

int main(void)
{
    xui_document document=NULL;
    assert(xuiDocumentCreateOwned(NULL,&document)==XUI_OK);
    xui_document_transaction txn=NULL;
    assert(xuiDocumentBeginTransaction(document,&edit,&txn)==XUI_OK);
    xui_doc_node_desc_t paragraph={.iSize=sizeof(paragraph),.iKind=XUI_DOC_PARAGRAPH};
    xui_doc_node_desc_t text={.iSize=sizeof(text),.iKind=XUI_DOC_TEXT,.sText="styled",.iTextBytes=6};
    uint64_t parent=0, node=0;
    assert(xuiDocumentTxnInsertNode(txn,XUI_DOCUMENT_ROOT,XUI_DOCUMENT_APPEND,&paragraph,&parent)==XUI_OK);
    char family[63];
    memset(family,'A',sizeof(family));
    xui_doc_node_attributes_view_t attrs={
        .tValues={.iMarks=XUI_DOC_BOLD,.iTextColor=0xEE4422FF,.fFontSize=18},
        .sFontFamily=family,.iFontFamilyBytes=sizeof(family),.sLanguage="zh-Hans"
    };
    assert(xuiDocumentTxnInsertNodeWithAttributes(txn,parent,XUI_DOCUMENT_APPEND,&attrs,&text,&node)==XUI_OK);
    memset(family,'B',sizeof(family));
    xui_doc_node_attributes_view_t result={0};
    assert(xuiDocumentTxnGetNodeAttributes(txn,node,&result)==XUI_OK);
    check_style(&result,sizeof(family));
    assert(result.sFontFamily[0]=='A' && result.sFontFamily[62]=='A');

    const char embedded[]={'A',0,'B'};
    xui_doc_node_attributes_view_t invalid=attrs;
    invalid.sFontFamily=embedded;
    invalid.iFontFamilyBytes=sizeof(embedded);
    uint64_t rejected=UINT64_MAX;
    assert(xuiDocumentTxnInsertNodeWithAttributes(txn,parent,XUI_DOCUMENT_APPEND,&invalid,&text,&rejected)==XUI_DOC_ERROR_SCHEMA && !rejected);
    assert(xuiDocumentTxnSetNodeAttributes(txn,node,&invalid)==XUI_DOC_ERROR_SCHEMA);
    invalid.iFontFamilyBytes=64;
    assert(xuiDocumentTxnSetNodeAttributes(txn,node,&invalid)==XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiDocumentTxnGetNodeAttributes(txn,node,&result)==XUI_OK && result.sFontFamily[0]=='A');

    const unsigned char payload[]={0x00,0xFF,0x00};
    xui_doc_node_desc_t extension={.iSize=sizeof(extension),.iKind=XUI_DOC_EXTENSION,
        .sResource="sample.node",.sInfo="binary",.pExtensionPayload=payload,
        .iExtensionPayloadBytes=sizeof(payload),.iExtensionVersion=7,.bExtensionRequired=1};
    const xui_doc_node_attributes_view_t empty={0};
    uint64_t extra=0;
    assert(xuiDocumentTxnInsertNodeWithAttributes(txn,XUI_DOCUMENT_ROOT,XUI_DOCUMENT_APPEND,&empty,&extension,&extra)==XUI_OK);
    xui_document_change_set changes=NULL;
    assert(xuiDocumentTxnCommit(txn,&changes)==XUI_OK && changes);
    assert(xuiDocumentChangeSetReleaseChecked(changes)==XUI_OK);
    assert(xuiDocumentTxnReleaseChecked(txn)==XUI_OK);

    xui_document_snapshot old=NULL;
    assert(xuiDocumentAcquireSnapshot(document,&old)==XUI_OK);
    unsigned char read[sizeof(payload)]={0};
    uint64_t used=0;
    assert(xuiDocumentSnapshotCopyExtensionPayload(old,extra,read,sizeof(read),&used)==XUI_OK);
    assert(used==sizeof(payload) && !memcmp(read,payload,sizeof(payload)));
    xui_doc_node_info_t extension_info={.iSize=sizeof(extension_info)};
    assert(xuiDocumentSnapshotGetNode(old,extra,&extension_info)==XUI_OK);
    assert(extension_info.iExtensionVersion==7 && extension_info.bExtensionRequired==1);
    assert(xuiDocumentSnapshotGetNodeAttributes(old,node,&result)==XUI_OK);
    check_style(&result,sizeof(family));
    assert(result.sFontFamily[0]=='A');

    int set_failures=check_write_failure(document,old->state->allocator,parent,node,&attrs,NULL);
    int insert_failures=check_write_failure(document,old->state->allocator,parent,node,&attrs,&text);
    assert(set_failures>0 && insert_failures>0);
    assert(xuiDocumentSnapshotGetNodeAttributes(old,node,&result)==XUI_OK && result.sFontFamily[0]=='A');

    assert(xuiDocumentBeginTransaction(document,&edit,&txn)==XUI_OK);
    attrs.sFontFamily="Next";
    attrs.iFontFamilyBytes=4;
    attrs.sLanguage="und";
    assert(xuiDocumentTxnSetNodeAttributes(txn,node,&attrs)==XUI_OK);
    assert(xuiDocumentTxnGetNodeAttributes(txn,node,&result)==XUI_OK && !strcmp(result.sLanguage,"und"));
    assert(xuiDocumentTxnCommit(txn,&changes)==XUI_OK);
    assert(xuiDocumentChangeSetReleaseChecked(changes)==XUI_OK);
    assert(xuiDocumentTxnReleaseChecked(txn)==XUI_OK);
    xui_document_snapshot newer=NULL;
    assert(xuiDocumentAcquireSnapshot(document,&newer)==XUI_OK);
    assert(xuiDocumentReleaseChecked(document)==XUI_OK);
    assert(xuiDocumentSnapshotGetNodeAttributes(old,node,&result)==XUI_OK && result.sFontFamily[0]=='A');
    assert(xuiDocumentSnapshotGetNodeAttributes(newer,node,&result)==XUI_OK && !strcmp(result.sFontFamily,"Next"));
    assert(xuiDocumentSnapshotGetNodeAttributes(newer,UINT64_MAX,&result)==XUI_ERROR_NOT_FOUND);
    assert(!strcmp(result.sFontFamily,"Next"));
    assert(xuiDocumentSnapshotReleaseChecked(newer)==XUI_OK);
    assert(xuiDocumentSnapshotReleaseChecked(old)==XUI_OK);
    printf("DOCUMENT_NODE_ATTRIBUTES_NATIVE_OK set-oom=%d insert-oom=%d\n",set_failures,insert_failures);
    return 0;
}
