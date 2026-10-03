#include "../src/xge_unicode_script.h"
#include "../lib/libunibreak/src/unibreakdef.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(e) do { if (!(e)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#e); exit(1); } } while(0)
static size_t decoded;
static uint32_t decode(const void* text, size_t length, size_t* at)
{ decoded++; return ub_get_next_char_utf8(text, length, at); }
static void check(const char* text, const char* tags)
{
    size_t length = strlen(text), at = 0, index = 0;
    unsigned char* map = malloc(length + 2); CHECK(map);
    memset(map, 0xcd, length + 2); decoded = 0;
    CHECK(__xgeScriptMap((const unsigned char*)text, length, map + 1, decode));
    CHECK(decoded <= length * 2 && map[0] == 0xcd && map[length + 1] == 0xcd);
    while (at < length) {
        size_t start = at; uint32_t cp = decode((const unsigned char*)text, length, &at), tag;
        (void)cp; CHECK(tags[index * 5] && strlen(tags) >= index * 5 + 4);
        tag = (uint32_t)(unsigned char)tags[index * 5] << 24 |
            (uint32_t)(unsigned char)tags[index * 5 + 1] << 16 |
            (uint32_t)(unsigned char)tags[index * 5 + 2] << 8 | (unsigned char)tags[index * 5 + 3];
        while (start < at) {
            if (__xgeScriptTag(map[start + 1]) != tag)
                fprintf(stderr,"scalar %zu text=%s got=%08x expected=%08x\n",index,text,__xgeScriptTag(map[start + 1]),tag);
            CHECK(__xgeScriptTag(map[start++ + 1]) == tag);
        }
        index++;
    }
    CHECK(strlen(tags) == (index ? index * 5 - 1 : 0)); free(map);
}
static void corpus(void)
{
    FILE* file = fopen("test_xui/data/unicode-17/GraphemeBreakTest.txt", "rb");
    char line[12000]; size_t cases = 0; CHECK(file);
    while (fgets(line, sizeof(line), file)) {
        unsigned char text[4096], map[4096]; unsigned offsets[4096], stops[4096];
        size_t bytes = 0, scalars = 0, i; char* token;
        char* comment = strchr(line, '#'); if (comment) *comment = 0;
        token = strtok(line, " \t\r\n");
        while (token) {
            int boundary = !strcmp(token, "\xc3\xb7");
            token = strtok(NULL, " \t\r\n"); if (!token) break;
            { unsigned long cp = strtoul(token, NULL, 16); CHECK(cp <= 0x10ffff);
              offsets[scalars] = (unsigned)bytes; stops[scalars++] = (unsigned)boundary;
              if (cp < 128) text[bytes++] = (unsigned char)cp;
              else if (cp < 0x800) { text[bytes++] = (unsigned char)(0xc0 | cp >> 6); text[bytes++] = (unsigned char)(0x80 | (cp & 63)); }
              else if (cp < 0x10000) { text[bytes++] = (unsigned char)(0xe0 | cp >> 12); text[bytes++] = (unsigned char)(0x80 | ((cp >> 6) & 63)); text[bytes++] = (unsigned char)(0x80 | (cp & 63)); }
              else { text[bytes++] = (unsigned char)(0xf0 | cp >> 18); text[bytes++] = (unsigned char)(0x80 | ((cp >> 12) & 63)); text[bytes++] = (unsigned char)(0x80 | ((cp >> 6) & 63)); text[bytes++] = (unsigned char)(0x80 | (cp & 63)); }
            }
            token = strtok(NULL, " \t\r\n");
        }
        if (!scalars) continue;
        CHECK(__xgeScriptMap(text, bytes, map, decode));
        for (i = 1; i < scalars; i++) if (!stops[i]) CHECK(map[offsets[i]] == map[offsets[i - 1]]);
        cases++;
    }
    CHECK(!ferror(file) && fclose(file) == 0 && cases == 766);
    printf("Script runs preserve all %zu official Unicode 17 grapheme cases\n", cases);
}
static void properties(const char* path)
{
    uint32_t count = (uint32_t)(sizeof(__xgeScriptTags) / sizeof(*__xgeScriptTags)), cp;
    FILE* file = fopen(path, "wb"); CHECK(file);
    CHECK(fwrite(&count, 4, 1, file) == 1 && fwrite(__xgeScriptTags, 4, count, file) == count);
    for (cp = 0; cp <= 0x10ffff; cp++) {
        xge_script_property_t p = __xgeScriptProperty(cp); uint32_t pair = 0;
        int bracket = __xgeScriptBracket(cp, &pair);
        CHECK(fputc(p.primary, file) != EOF && fwrite(__xgeScriptSets[p.extensions].words, 8, 3, file) == 3);
        CHECK(fwrite(&pair, 4, 1, file) == 1 && fputc(bracket + 1, file) != EOF);
    }
    CHECK(fclose(file) == 0);
}
int main(int argc, char** argv)
{
    check("", ""); check("123!?", "Zyyy Zyyy Zyyy Zyyy Zyyy");
    check(" (a)", "Latn Latn Latn Latn");
    check("a(\xce\xbb)a", "Latn Latn Grek Latn Latn");
    check("(\xce\xbb)a", "Grek Grek Grek Latn");
    check("a\xe3\x83\xbc\xe3\x82\xab", "Latn Kana Kana");
    check("\xe3\x83\xbc\xe3\x82\xab", "Kana Kana");
    check("a\xe3\x83\xbc\xe3\x81\x82", "Latn Hira Hira");
    check("\xe3\x81\x82\xe3\x83\xbc" "a", "Hira Hira Latn");
    check("\xdc\x90\xd9\x80\xdc\x90", "Syrc Syrc Syrc");
    check("\xd9\x80\xdc\x90", "Syrc Syrc");
    check("\xd9\x80\xd8\xa8", "Arab Arab");
    check("a\xd9\x80\xdc\x90", "Latn Syrc Syrc");
    check("\xce\xbb\xe2\x97\x8f\xd6\xb0", "Grek Hebr Hebr");
    check("a\xd6\xb0", "Latn Latn");
    check("a\xe2\x80\x8d" "b", "Latn Latn Latn");
    check("a\n(\xce\xbb)a", "Latn Zyyy Grek Grek Grek Latn");
    check("a\r\n\xce\xbb", "Latn Zyyy Zyyy Grek");
    check("a\xe2\x80\xa9\xce\xbb", "Latn Zyyy Grek");
    check("a\xe2\x8c\xa9\xce\xbb\xe3\x80\x89", "Latn Latn Grek Latn");
    check("a\xe3\x80\x88\xce\xbb\xe3\x80\x89", "Latn Latn Grek Latn");
    check("\xe3\x80\x88\xce\xbb\xe3\x80\x89" "a", "Grek Grek Grek Latn");
    check("a(\xce\xbb[\xd0\xb0])", "Latn Latn Grek Grek Cyrl Grek Latn");
    corpus();
    {
        size_t length = 200000, i; unsigned char* text = malloc(length), *map = malloc(length); CHECK(text && map);
        for (i = 0; i < length; i++) text[i] = i % 2 ? ')' : '(';
        decoded = 0; CHECK(__xgeScriptMap(text, length, map, decode) && decoded <= length * 2);
        for (i = 0; i < length; i++) CHECK(map[i] == 0);
        memset(text, 'a', length); text[0] = '(';
        decoded = 0; CHECK(__xgeScriptMap(text, length, map, decode) && decoded <= length * 2);
        for (i = 0; i < length; i++) CHECK(__xgeScriptTag(map[i]) == 0x4c61746eu);
        free(text); free(map);
    }
    {
        size_t nested = 20000, length = nested * 2 + 3, i;
        unsigned char* text = malloc(length), *map = malloc(length); CHECK(text && map);
        text[0] = 'a'; memset(text + 1, '(', nested);
        text[nested + 1] = 0xce; text[nested + 2] = 0xbb;
        memset(text + nested + 3, ')', nested);
        decoded = 0; CHECK(__xgeScriptMap(text, length, map, decode) && decoded <= length * 2);
        for (i = 0; i < length; i++) CHECK(__xgeScriptTag(map[i]) ==
            (i == nested + 1 || i == nested + 2 ? 0x4772656bu : 0x4c61746eu));
        free(text); free(map);
    }
    { unsigned char map[4]; CHECK(!__xgeScriptMap((const unsigned char*)"\xf0\x80", 2, map, decode)); }
    if (argc == 3 && !strcmp(argv[1], "--properties")) properties(argv[2]); else CHECK(argc == 1);
    puts("Actual C Script_Extensions: prefix/intersection/primary overrides/bracket nesting/canonical pairs/paragraph resets, bounded linear scans and truncated input rejection passed");
    return 0;
}
