/* Private libunibreak 7.0 inclusion. See lib/libunibreak/README.xui.md.
 * Keep upstream sources unmodified and their external names out of XUI's ABI. */
#define unibreak_version __xuiUbVersion
#define ub_get_next_char_utf8 __xuiUbNext8
#define ub_get_next_char_utf16 __xuiUbNext16
#define ub_get_next_char_utf32 __xuiUbNext32
#define ub_bsearch __xuiUbSearch
#define lb_prop_supplementary __xuiUbSupplementary
#define lb_prop_supplementary_len __xuiUbSupplementaryCount
#define lb_prop_bmp __xuiUbBmp
#define lb_prop_lang_map __xuiUbLanguages
#define lb_init_break_context __xuiUbInit
#define lb_process_next_char __xuiUbProcess
#define lb_get_char_class __xuiUbClass
#define set_linebreaks __xuiUbLineBreaks
#define init_linebreak __xuiUbInitLine
#define set_linebreaks_utf8 __xuiUbLine8
#define set_linebreaks_utf16 __xuiUbLine16
#define set_linebreaks_utf32 __xuiUbLine32
#define set_linebreaks_utf8_per_code_point __xuiUbLine8Points
#define set_linebreaks_utf16_per_code_point __xuiUbLine16Points
#define is_line_breakable __xuiUbPair
#define ub_get_char_eaw_class __xuiUbEastAsianWidth
#define ub_is_op_east_asian __xuiUbEastAsianOpen
#define ub_is_extended_pictographic __xuiUbPictographic
#define init_graphemebreak __xuiUbInitGrapheme
#define set_graphemebreaks_utf8 __xuiUbGrapheme8
#define set_graphemebreaks_utf16 __xuiUbGrapheme16
#define set_graphemebreaks_utf32 __xuiUbGrapheme32

#include "../lib/libunibreak/src/unibreakbase.c"
#include "../lib/libunibreak/src/unibreakdef.c"
#include "../lib/libunibreak/src/linebreakdata.c"
#include "../lib/libunibreak/src/linebreakdef.c"
#include "../lib/libunibreak/src/eastasianwidthdef.c"
#include "../lib/libunibreak/src/emojidef.c"
#include "../lib/libunibreak/src/linebreak.c"
#include "../lib/libunibreak/src/graphemebreak.c"

#undef unibreak_version
#undef ub_get_next_char_utf8
#undef ub_get_next_char_utf16
#undef ub_get_next_char_utf32
#undef ub_bsearch
#undef lb_prop_supplementary
#undef lb_prop_supplementary_len
#undef lb_prop_bmp
#undef lb_prop_lang_map
#undef lb_init_break_context
#undef lb_process_next_char
#undef lb_get_char_class
#undef set_linebreaks
#undef init_linebreak
#undef set_linebreaks_utf8
#undef set_linebreaks_utf16
#undef set_linebreaks_utf32
#undef set_linebreaks_utf8_per_code_point
#undef set_linebreaks_utf16_per_code_point
#undef is_line_breakable
#undef ub_get_char_eaw_class
#undef ub_is_op_east_asian
#undef ub_is_extended_pictographic
#undef init_graphemebreak
#undef set_graphemebreaks_utf8
#undef set_graphemebreaks_utf16
#undef set_graphemebreaks_utf32

#ifndef XUI_TEXT_BREAK_TEST_COUNT
#define XUI_TEXT_BREAK_TEST_COUNT(field, count) ((void)0)
#endif

enum {
	XUI_LB_GRAPHEME = 1,
	XUI_LB_NORMAL = 2,
	XUI_LB_EMERGENCY = 4,
	XUI_LB_HARD = 8,
	XUI_LB_INVISIBLE = 16,
	XUI_LB_SOFT_HYPHEN = 32,
	XUI_LB_HYPHEN_USED = 64
};

static utf32_t __xuiTextBreakDecode(const void* pText, size_t iSize, size_t* pAt)
{
	xstrview tView;
	size_t iRead = 0;
	uint32 iScalar = 0;
	XUI_TEXT_BREAK_TEST_COUNT(iDecode, 1);
	if ( *pAt >= iSize ) return EOS;
	tView.Data = (const char*)pText + *pAt;
	tView.Size = iSize - *pAt;
	if ( xrtUtf8Decode(tView, &iScalar, &iRead) != XUTF_OK ) {
		/* Match the existing text fallback: each malformed byte makes progress. */
		iScalar = 0xfffdu;
		iRead = 1;
	}
	*pAt += iRead;
	return (utf32_t)iScalar;
}

static int __xuiTextBreakLetter(enum LineBreakClass iClass)
{
	return iClass == LBP_AL || iClass == LBP_HL || iClass == LBP_NU;
}

static int __xuiTextBreakMap(const char* sText, int iSize, unsigned char* pMap)
{
	struct LineBreakContext tContext;
	enum LineBreakClass iPrevious = LBP_Undefined;
	char* pWork;
	size_t iAt = 0, i;
	if ( iSize <= 0 ) return XUI_OK;
	if ( (size_t)iSize > SIZE_MAX / 2u ) return XUI_ERROR_OUT_OF_MEMORY;
	pWork = (char*)xrtMalloc((size_t)iSize * 2u);
	if ( pWork == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	__xuiUbLineBreaks(sText, (size_t)iSize, "-strict", LBOT_PER_CODE_UNIT,
		pWork, __xuiTextBreakDecode);
	set_graphemebreaks(sText, (size_t)iSize, pWork + iSize, __xuiTextBreakDecode);
	for ( i = 1; i <= (size_t)iSize; i++ ) {
		XUI_TEXT_BREAK_TEST_COUNT(iMap, 1);
		if ( pWork[iSize + i - 1] == GRAPHEMEBREAK_BREAK ) {
			pMap[i] |= XUI_LB_GRAPHEME;
			if ( pWork[i - 1] != LINEBREAK_NOBREAK && pWork[i - 1] != LINEBREAK_INSIDEACHAR ) pMap[i] |= XUI_LB_NORMAL;
		}
	}
	xrtFree(pWork);
	__xuiUbInit(&tContext, 0, "-strict");
	while ( iAt < (size_t)iSize ) {
		size_t iStart = iAt, j;
		utf32_t iScalar = __xuiTextBreakDecode(sText, (size_t)iSize, &iAt);
		enum LineBreakClass iClass = resolve_lb_class(__xuiUbClass(&tContext, iScalar), &tContext);
		XUI_TEXT_BREAK_TEST_COUNT(iProperties, 1);
		/* Tailoring is class-based, never a hand-maintained punctuation list. */
		if ( (pMap[iStart] & XUI_LB_GRAPHEME) && __xuiTextBreakLetter(iPrevious) &&
		     __xuiTextBreakLetter(iClass) ) pMap[iStart] |= XUI_LB_EMERGENCY;
		if ( iClass == LBP_BK || iClass == LBP_NL || iClass == LBP_CR || iClass == LBP_LF ) pMap[iStart] |= XUI_LB_HARD;
		if ( iScalar == 0xadu ) pMap[iAt] |= XUI_LB_SOFT_HYPHEN;
		if ( iScalar == 0xadu || iScalar == 0x200bu || iScalar == 0x2060u || iScalar == 0xfeffu ) {
			for ( j = iStart; j < iAt; j++ ) pMap[j] |= XUI_LB_INVISIBLE;
		}
		if ( iClass != LBP_CM ) iPrevious = iClass;
	}
	return XUI_OK;
}
