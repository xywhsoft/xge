#include "../xui_document.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "public link tokens line %d: %s\n", __LINE__, #c); exit(1); } } while (0)
typedef struct fault_allocator{long remaining;unsigned live;}fault_allocator;
static void* fault_alloc(void* user,size_t bytes){fault_allocator* a=user;void* p;if(!a->remaining)return NULL;if(a->remaining>0)a->remaining--;p=malloc(bytes);if(p)a->live++;return p;}
static void fault_free(void* user,void* p){fault_allocator* a=user;CHECK(a->live);a->live--;free(p);}
static char* source_variant(const char* source,unsigned eol,unsigned bom,uint64_t* size)
{
    size_t n=strlen(source),used=0,i;char* out=malloc(n*2+4);CHECK(out);
    if(bom){memcpy(out,"\xef\xbb\xbf",3);used=3;}
    for(i=0;i<n;i++){if(source[i]=='\n'){if(eol)out[used++]='\r';if(eol!=2)out[used++]='\n';}else out[used++]=source[i];}
    out[used]=0;*size=used;return out;
}
static xui_document open_doc(const char* source,uint64_t bytes,unsigned dialect,fault_allocator* a)
{
    xui_doc_desc_t desc={0};xui_document d;int result;
    desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;desc.iMarkdownDialect=dialect;
    if(a){desc.onAlloc=fault_alloc;desc.onFree=fault_free;desc.pAllocatorUser=a;}
    CHECK(xuiDocumentCreate(&desc,&d)==XUI_OK);result=xuiDocumentLoadMarkdown(d,source,bytes);
    if(result!=XUI_OK)fprintf(stderr,"load result %d, input %s\n",result,source);
    CHECK(result==XUI_OK);return d;
}
static xui_doc_source_binding_t required(xui_document_source_index index, uint32_t kind, uint32_t detail,
    uint64_t start, uint64_t end, uint32_t record_flags)
{
    xui_doc_source_index_info_t info = {0}; xui_doc_source_binding_t match = {0}; uint64_t i, count = 0;
    info.iSize = sizeof(info); CHECK(xuiDocumentSourceIndexGetInfo(index, &info) == XUI_OK);
    for (i = 0; i < info.iBindingCount; i++) {
        xui_doc_source_binding_t b = {0}; xui_doc_source_record_t r = {0}; b.iSize = sizeof(b); r.iSize = sizeof(r);
        CHECK(xuiDocumentSourceIndexGetBinding(index, i, &b) == XUI_OK);
        if (b.iOriginKind != XUI_DOC_SOURCE_BINDING_DEFINITION_SYNTAX || b.tRange.iKind != kind ||
            b.tRange.iDetail != detail || b.tRange.iSourceStart != start || b.tRange.iSourceEnd != end) continue;
        CHECK(xuiDocumentSourceIndexGetRecord(index, b.tRange.iRecordIndex, &r) == XUI_OK);
        if (record_flags && (r.iFlags & record_flags) != record_flags) continue;
        CHECK(b.iMetadata & XUI_DOC_SOURCE_BINDING_SOURCE_ONLY);
        CHECK(!b.tRange.iNodeId && b.tRange.iTextStart == UINT64_MAX && b.tRange.iTextEnd == UINT64_MAX);
        CHECK(b.tRange.iFlags & XUI_DOC_SOURCE_TOKEN_PARSER_CONFIRMED); count++; match = b;
    }
    if (count != 1) fprintf(stderr, "binding role %u detail %u [%llu,%llu] flags %u matches %llu\n",kind,detail,
        (unsigned long long)start,(unsigned long long)end,record_flags,(unsigned long long)count);
    CHECK(count == 1); return match;
}
static void partition_role(xui_document_source_index index, uint64_t start, uint64_t end,
    uint32_t kind, uint32_t detail)
{
    uint64_t at;
    for (at = start; at < end; at++) {
        uint64_t ordinal; xui_doc_source_token_t token = {0}; token.iSize = sizeof(token);
        CHECK(xuiDocumentSourceIndexFindToken(index, at, &ordinal) == XUI_OK &&
            xuiDocumentSourceIndexGetToken(index, ordinal, &token) == XUI_OK);
        CHECK(token.iKind == kind && token.iDetail == detail &&
            token.iSourceStart <= at && at < token.iSourceEnd && !token.iNodeId &&
            token.iTextStart == UINT64_MAX && token.iTextEnd == UINT64_MAX);
    }
}
int main(void)
{
    const char* fixtures[] = {"[a]: <uri> \"title\" \t\n[a]\n", "[a]: <> ()\n[a]\n",
        "> [a]:\n> \t<uri>\n> \t'title'\n>\n> [a]\n", "[a]: bare(thing) 'title'\n[a]\n"};
    unsigned f, dialect, eol, bom, total = 0;
    for (f = 0; f < 4; f++) for (dialect = 1; dialect <= 3; dialect++)
    for (eol = 0; eol < 3; eol++) for (bom = 0; bom < 2; bom++) {
        fault_allocator a = {-1, 0}; uint64_t bytes, label, uri, title, ubytes, tbytes, colon;
        char* source = source_variant(fixtures[f], eol, bom, &bytes);
        xui_document d = open_doc(source, bytes, dialect, &a); xui_document_snapshot s; xui_document_source_index index;
        CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK && xuiDocumentSnapshotCreateSourceIndex(s, NULL, &index) == XUI_OK);
        label = (uint64_t)(strstr(source, "[a]")-source); colon = label+3;
        uri = (uint64_t)(strstr(source, f == 3 ? "bare(thing)" : f == 1 ? "<>" : "<uri>")-source);
        title = (uint64_t)(strstr(source, f == 0 ? "\"title\"" : f == 1 ? "()" : "'title'")-source);
        ubytes = f == 3 ? 11 : f == 1 ? 2 : 5; tbytes = f == 1 ? 2 : 7;
        a.remaining = 0;
        required(index, XUI_DOC_SOURCE_TOKEN_REFERENCE_LABEL, XUI_DOC_REFERENCE_LINK, label, label+3, 0);
        required(index, XUI_DOC_SOURCE_TOKEN_REFERENCE_LABEL, XUI_DOC_REFERENCE_LINK, label+1, label+2, 0);
        required(index, XUI_DOC_SOURCE_TOKEN_REFERENCE_SYNTAX, XUI_DOC_REFERENCE_LINK, label, label+1, XUI_DOC_SOURCE_RECORD_SYNTAX_OPEN);
        required(index, XUI_DOC_SOURCE_TOKEN_REFERENCE_SYNTAX, XUI_DOC_REFERENCE_LINK, label+2, label+3, XUI_DOC_SOURCE_RECORD_SYNTAX_CLOSE);
        required(index, XUI_DOC_SOURCE_TOKEN_REFERENCE_SYNTAX, XUI_DOC_REFERENCE_LINK, colon, colon+1, XUI_DOC_SOURCE_RECORD_SYNTAX_DEFINITION_SEPARATOR);
        required(index, XUI_DOC_SOURCE_TOKEN_INLINE_MARKER, XUI_DOC_SYNTAX_DESTINATION, uri, uri+ubytes, 0);
        required(index, XUI_DOC_SOURCE_TOKEN_REFERENCE_DESTINATION, XUI_DOC_REFERENCE_LINK,
            uri+(f != 3), uri+ubytes-(f != 3), 0);
        partition_role(index, uri+(f != 3), uri+ubytes-(f != 3),
            XUI_DOC_SOURCE_TOKEN_REFERENCE_DESTINATION, XUI_DOC_REFERENCE_LINK);
        if (f != 3) {
            required(index, XUI_DOC_SOURCE_TOKEN_INLINE_MARKER, XUI_DOC_SYNTAX_DESTINATION, uri, uri+1, XUI_DOC_SOURCE_RECORD_SYNTAX_OPEN);
            required(index, XUI_DOC_SOURCE_TOKEN_INLINE_MARKER, XUI_DOC_SYNTAX_DESTINATION, uri+ubytes-1, uri+ubytes, XUI_DOC_SOURCE_RECORD_SYNTAX_CLOSE);
        }
        required(index, XUI_DOC_SOURCE_TOKEN_INLINE_MARKER, XUI_DOC_SYNTAX_TITLE, title, title+tbytes, 0);
        required(index, XUI_DOC_SOURCE_TOKEN_REFERENCE_TITLE, XUI_DOC_REFERENCE_LINK, title+1, title+tbytes-1, 0);
        partition_role(index, title+1, title+tbytes-1, XUI_DOC_SOURCE_TOKEN_REFERENCE_TITLE, XUI_DOC_REFERENCE_LINK);
        required(index, XUI_DOC_SOURCE_TOKEN_INLINE_MARKER, XUI_DOC_SYNTAX_TITLE, title, title+1, XUI_DOC_SOURCE_RECORD_SYNTAX_OPEN);
        required(index, XUI_DOC_SOURCE_TOKEN_INLINE_MARKER, XUI_DOC_SYNTAX_TITLE, title+tbytes-1, title+tbytes, XUI_DOC_SOURCE_RECORD_SYNTAX_CLOSE);
        if (f == 2) {
            xui_doc_source_binding_t prefix = required(index, XUI_DOC_SOURCE_TOKEN_CONTAINER_INDENT, XUI_DOC_REFERENCE_LINK,
                uri-3, uri, XUI_DOC_SOURCE_RECORD_SYNTAX_DEFINITION_PREFIX);
            CHECK((prefix.iMetadata & XUI_DOC_SOURCE_BINDING_COLUMNS) && !prefix.iStartColumn &&
                prefix.iContentColumn == 4 && prefix.iEndColumn == 4);
            required(index, XUI_DOC_SOURCE_TOKEN_HORIZONTAL_SPACE, 0, uri-2, uri, 0);
        }
        if (f == 0) required(index, XUI_DOC_SOURCE_TOKEN_HORIZONTAL_SPACE, 0, title+7, title+9, 0);
        {
            const char* ending = source+title+tbytes+(f == 0 ? 2 : 0);
            uint64_t ending_bytes = eol == 1 ? 2 : 1;
            required(index, XUI_DOC_SOURCE_TOKEN_LINE_ENDING, 0, ending-source, ending-source+ending_bytes,
                XUI_DOC_SOURCE_RECORD_SYNTAX_BREAK_ENDING);
        }
        a.remaining = -1; xuiDocumentRelease(d); xuiDocumentSnapshotRelease(s);
        { char raw[3]; CHECK(xuiDocumentSourceIndexReadSource(index, label, raw, 3) == XUI_OK && !memcmp(raw,"[a]",3)); }
        xuiDocumentSourceIndexRelease(index); CHECK(!a.live); free(source); total++;
    }
    printf("Link syntax public goldens: %u independent delimiter/label/colon/URI/title/empty-anchor/space/ending/Tab-column cases; reference binding and flat partition roles agree, no query allocations or semantic positions, retained index lifetime and zero leaks\n", total);
    return 0;
}
