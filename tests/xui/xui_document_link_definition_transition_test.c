#define DOC_CST_TRANSITION_NO_MAIN
#include "xui_document_cst_transition_test.c"
static const char* link_transitions[] = {
    "[a]: <uri> \"title\"\n\n[a]\n",
    "[ a b ]:\tbare(thing)\t'title'  \n[a b]\n",
    "[a]: <uri>\n  \"first\nlast\"\n\n[a]\n",
    "[a\n b]: <> ()\n[a b]\n",
    "[a]: <first> \"x\"\n[a]: <uri> 'title'\n[a]\n",
    "head\n\n[unused]: uri\n",
    "> - [a]: <uri> (title)\n>   [a]\n",
    "> [a]:\n> \t<uri>\n> \t'title'\n>\n> [a]\n",
    "[^n]: [a]: <uri> \"title\"\n\nuse[^n]\n",
    "top\n\n[^n]: [a]: <uri> 'title'\n",
    "[a\\]b]: <u\\>ri> \"t\\\"itle\"\n[a\\]b]\n",
    "[空 白]: <中文> (标题)\n[空 白]\n",
    "[a]: <>\n[a]: <two>\n\n[^n]: **body** [a]\ntext[^n]\n",
    "[a]: <uri>\n[^n]: body [a]\n[^n]: duplicate\n\n[a] [^n]\n",
    "- item\n\n  [a]: <uri> \"title\"\n\n  [a]\n- next\n"
};
int main(void)
{
    unsigned f, eol, bom, op, mode; uint64_t total = 0;
    (void)transitions;
    for (f = 0; f < sizeof(link_transitions)/sizeof(*link_transitions); f++)
    for (eol = 0; eol < 3; eol++) for (bom = 0; bom < 2; bom++) {
        uint64_t n, at; char* source = source_variant(link_transitions[f], eol, bom, &n);
        for (at = bom ? 3 : 0; at < n; at++) {
            if (((unsigned char)source[at] & 0xc0) == 0x80) continue;
            for (op = 0; op < 3; op++) for (mode = 0; mode < 2; mode++) {
                fprintf(stderr, "Link transition f=%u eol=%u bom=%u at=%llu op=%u mode=%u\n", f, eol, bom, (unsigned long long)at, op, mode);
                transition_case(source, n, at, op, mode); total++;
            }
        }
        free(source);
    }
    printf("Link CST transitions: %llu SOURCE/Prepare insert/delete/replace; exact syntax, binding, HTML, full parse, saved history and lifetime passed\n", (unsigned long long)total);
    return 0;
}
