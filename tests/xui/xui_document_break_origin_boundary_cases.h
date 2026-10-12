static void origin_boundary_differential(void)
{
    static const char* bodies[] = {
        "alpha *emphasis*\n\nbeta **strong**\n",
        "use [missing]\n\nplain\n",
        "use [^new]\n\nplain\n",
        "```c\n[a] plain\n```\n\ntext\n",
        "<!-- comment\ninside -->\n\ntext <span>inline</span>\n",
        "<div>\nraw html\n</div>\n\ntext\n",
        "> quoted\n>\n> - first\n> - second\n\ntext\n",
        "- [ ] task\n\n- next\n\ntext\n",
        "| A | B |\n| :-- | --: |\n| x | `|` |\n\ntext\n",
        "$$\na+b\n$$\n\ntext $c+d$\n",
        "> [!NOTE]\n> original\n\ntext\n",
        "title\n=====\n\ntext\n"
    };
    static const char* edits[] = {
        "x", "", "\n", "\r\n", "[", "]", "[^n]", "[r]: /new\n",
        "[^n]: first [^m]\n\n[^m]: next\n", "```\n", "---\n", "<!--", "-->", "> ", "\t", "|"
    };
    uint64_t cases = 0, incremental = 0; unsigned f, e, dialect; size_t at;
    for (dialect = XUI_MD_COMMONMARK; dialect <= XUI_MD_EXTENDED; dialect++)
    for (f = 0; f < sizeof(bodies) / sizeof(*bodies); f++) {
        const char* prefix = "first *kept* &amp;\n\n## second kept\n\nthird `kept`\n\n";
        char source[2048];
        snprintf(source, sizeof(source), "%s%s\nlast kept *span*\n\n## final kept\n", prefix, bodies[f]);
        for (at = 0; at <= strlen(bodies[f]); at += 3)
        for (e = 0; e < sizeof(edits) / sizeof(*edits); e++) {
            uint64_t remove = (e & 1) && at < strlen(bodies[f]) ? 1 : 0;
            incremental += inc_case(source, dialect, strlen(prefix) + at, remove, edits[e], (int)(cases & 1));
            cases++;
        }
    }
    CHECK(incremental > cases / 3 && incremental <= cases);
    printf("General no-definition boundary probe: %llu cases, %llu incremental; three dialects, fence/HTML/container/table/math/admonition boundaries, full semantic/source/CST/definitions/Undo oracle passed\n",
        (unsigned long long)cases, (unsigned long long)incremental);
}
