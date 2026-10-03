#include "../src/xge_text_context.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(e) do { if (!(e)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#e); exit(1); } } while(0)
int main(void)
{
    const char* valid[] = {NULL,"und","en","EN-us","tr-TR","zh-Hant-TW","de-CH-1901","sl-rozaj-biske-1994",
        "en-a-aaa-b-bbb-x-a","zh-cmn-Hans-CN","ar-aao","en-123","abcd","abcde","abcdefgh","x-a","X-a-1",
        "en-0-abc","i-klingon","EN-gb-OED","sgn-BE-FR","zh-min-nan","en-abc-def-ghi-Latn-US-1234-varia-a-abc-x-z"};
    const char* invalid[] = {"","a","i-unknown","en_US","en-","-en","en--US","en-abcdefghi","123","en-1abc-1ABC",
        "sl-rozaj-ROZAJ","en-a-aa-A-bb","en-a","en-a-b-aaa","x","en-x","en-12","en-US-GB","en-abc-def-ghi-jkl",
        "en-abcde-US","en-Latn-Cyrl","en-1234-1234","en-\xc3\xa9","en-@","en-9"};
    xge_text_shape_desc_t input = {0}, output = {0}; size_t i; char tag[258];
    for (i=0;i<sizeof(valid)/sizeof(*valid);i++) CHECK(__xgeTextLanguageValid(valid[i]));
    for (i=0;i<sizeof(invalid)/sizeof(*invalid);i++) CHECK(!__xgeTextLanguageValid(invalid[i]));
    /* The implementation accepts 255 ASCII bytes, rejects a longer tag, and
     * accepts arbitrary syntactically valid private-use subtags without IANA. */
    memcpy(tag,"x",1); for(i=1;i<255;i+=2){tag[i]='-';tag[i+1]='a';} tag[255]=0;
    CHECK(__xgeTextLanguageValid(tag)); tag[255]='b';tag[256]=0;CHECK(!__xgeTextLanguageValid(tag));
    input.sText="\xd8\xa8"; CHECK(__xgeTextContextNormalize(&input,2,&output)==XGE_OK && output.iContextSize==2);
    input.sContext="a\xd8\xa8z";input.iContextSize=-1;input.iContextOffset=1;
    CHECK(__xgeTextContextNormalize(&input,2,&output)==XGE_OK && output.iContextSize==4 && output.iContextOffset==1);
    input.iContextOffset=2;CHECK(__xgeTextContextNormalize(&input,2,&output)==XGE_ERROR_INVALID_ARGUMENT);
    input.iContextOffset=1;CHECK(__xgeTextContextNormalize(&input,1,&output)==XGE_ERROR_INVALID_ARGUMENT);
    input.iContextOffset=INT_MAX;CHECK(__xgeTextContextNormalize(&input,2,&output)==XGE_ERROR_INVALID_ARGUMENT);
    input.iContextOffset=-1;CHECK(__xgeTextContextNormalize(&input,2,&output)==XGE_ERROR_INVALID_ARGUMENT);
    input.iContextOffset=1;input.iContextSize=1;CHECK(__xgeTextContextNormalize(&input,2,&output)==XGE_ERROR_INVALID_ARGUMENT);
    input.iContextSize=-2;CHECK(__xgeTextContextNormalize(&input,2,&output)==XGE_ERROR_INVALID_ARGUMENT);
    input.iContextSize=4;input.sText="aa";CHECK(__xgeTextContextNormalize(&input,2,&output)==XGE_ERROR_INVALID_ARGUMENT);
    input.sText="\xd8\xa8";input.iScript=UINT32_C(0x53797263);input.sLanguage="ar";
    CHECK(__xgeTextContextNormalize(&input,2,&output)==XGE_OK);
    input.iScript=UINT32_C(0x53797230);CHECK(__xgeTextContextNormalize(&input,2,&output)==XGE_ERROR_INVALID_ARGUMENT);
    input.iScript=0;input.sLanguage="en_US";CHECK(__xgeTextContextNormalize(&input,2,&output)==XGE_ERROR_INVALID_ARGUMENT);
    memset(&input,0,sizeof(input));input.sText="a";input.iContextSize=1;
    CHECK(__xgeTextContextNormalize(&input,1,&output)==XGE_ERROR_INVALID_ARGUMENT);
    input.iContextSize=0;input.iContextOffset=1;CHECK(__xgeTextContextNormalize(&input,1,&output)==XGE_ERROR_INVALID_ARGUMENT);
    input.iContextOffset=0;CHECK(__xgeTextContextNormalize(&input,0,&output)==XGE_OK && !output.iContextSize);
    puts("Text context C contract: language grammar/case/duplicates/255-byte bound, item-byte equality, scalar boundaries and overflow passed");
    return 0;
}
