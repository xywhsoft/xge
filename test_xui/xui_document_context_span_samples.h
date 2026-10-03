#ifdef TEST_RTL_CONTEXT
#define SPAN_BODY "\xd7\x90 \xd7\x91"
#define SPAN_MARKDOWN "**\xd7\x90** *\xd7\x91*"
#define SPAN_LIGATURE "\xd7\x92\xd7\x93"
#define SPAN_FONT "test/data/xge_rtl_context_fixture.ttf"
#define SPAN_SCALAR_BYTES 2
#define SPAN_NAME "RTL Hebrew"
#define SPAN_DIRECTION XUI_TEXT_SHAPE_RTL
#define SPAN_PUA_CONTEXT_FIRST "\xee\x98\x84"
#define SPAN_PUA_CONTEXT_LAST "\xee\x98\x85"
#define SPAN_PUA_LIG_FIRST "\xee\x98\x86"
#define SPAN_PUA_LIG_LAST "\xee\x98\x87"
#define SPAN_PUA_LIGATURE "\xee\x98\x88"
#elif defined(TEST_UNICODE_CONTEXT)
#define SPAN_BODY "\xce\xbb \xce\xbc"
#define SPAN_MARKDOWN "**\xce\xbb** *\xce\xbc*"
#define SPAN_LIGATURE "\xcf\x86\xce\xb1"
#define SPAN_FONT "test/data/xge_unicode_context_fixture.ttf"
#define SPAN_SCALAR_BYTES 2
#define SPAN_NAME "Unicode"
#define SPAN_PUA_CONTEXT_FIRST "\xee\x94\x84"
#define SPAN_PUA_CONTEXT_LAST "\xee\x94\x85"
#define SPAN_PUA_LIG_FIRST "\xee\x94\x86"
#define SPAN_PUA_LIG_LAST "\xee\x94\x87"
#define SPAN_PUA_LIGATURE "\xee\x94\x88"
#else
#define SPAN_BODY "a b"
#define SPAN_MARKDOWN "**a** *b*"
#define SPAN_LIGATURE "fi"
#define SPAN_FONT "test/data/xge_ascii_context_fixture.ttf"
#define SPAN_SCALAR_BYTES 1
#define SPAN_NAME "ASCII"
#define SPAN_PUA_CONTEXT_FIRST "\xee\x90\x84"
#define SPAN_PUA_CONTEXT_LAST "\xee\x90\x85"
#define SPAN_PUA_LIG_FIRST "\xee\x90\x86"
#define SPAN_PUA_LIG_LAST "\xee\x90\x87"
#define SPAN_PUA_LIGATURE "\xee\x90\x88"
#endif
#ifndef SPAN_DIRECTION
#define SPAN_DIRECTION 0
#endif
static const unsigned span_offsets[]={0,SPAN_SCALAR_BYTES,SPAN_SCALAR_BYTES+1,2*SPAN_SCALAR_BYTES+1};
