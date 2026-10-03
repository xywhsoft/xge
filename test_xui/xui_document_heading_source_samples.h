#ifndef XUI_DOCUMENT_HEADING_SOURCE_SAMPLES_H
#define XUI_DOCUMENT_HEADING_SOURCE_SAMPLES_H
static const struct { const char* input; unsigned level; const char* expected; const char* raw_content; } heading_source_samples[] = {
    {"## \t**abcd** &amp; ### \t\r\n", 4, "#### \t**abcd** &amp; ### \t\r\n", "**abcd** &amp;"},
    {"abcd &amp;\r\n==== \t\r\n", 2, "abcd &amp;\r\n---- \t\r\n", "abcd &amp;"},
    {"> abcd &amp;\r\n> ==== \t\r\n", 0, "> abcd &amp;\r\n", "abcd &amp;"},
    {"abcd &amp;\n----- \t\n", 3, "### abcd &amp;\n", "abcd &amp;"},
    {"- abcd &amp;\n  ==== \t\n", 5, "- ##### abcd &amp;\n", "abcd &amp;"},
    {"> ## \t**abcd** &amp; ### \t\r\n", 0, "> **abcd** &amp;\r\n", "**abcd** &amp;"},
    {"  ##  abcd &amp;  \r", 1, "  #  abcd &amp;  \r", "abcd &amp;"},
    {"\xef\xbb\xbf## abcd &amp; ###", 6, "\xef\xbb\xbf###### abcd &amp; ###", "abcd &amp;"},
    {"abcd &amp;\r====", 2, "abcd &amp;\r----", "abcd &amp;"},
    {"abcd &amp;\r----", 0, "abcd &amp;\r", "abcd &amp;"},
    {"> abcd\n> efgh &amp;\n> ===\n", 2, "> abcd\n> efgh &amp;\n> ---\n", "abcd\n> efgh &amp;"},
    {"> abcd\n> efgh &amp;\n> ===\n", 0, "> abcd\n> efgh &amp;\n", "abcd\n> efgh &amp;"},
    {"- > abcd &amp;\r\n  > ==== \t\r\n", 4, "- > #### abcd &amp;\r\n", "abcd &amp;"},
    {"[^n]: ## \t**abcd** &amp; ### \t\r\n\r\nuse [^n]\r\n", 0,
     "[^n]: **abcd** &amp;\r\n\r\nuse [^n]\r\n", "**abcd** &amp;"},
    {"[^n]: abcd &amp;\r\n    ==== \t\r\n\r\nuse [^n]\r\n", 2,
     "[^n]: abcd &amp;\r\n    ---- \t\r\n\r\nuse [^n]\r\n", "abcd &amp;"},
    {"[^n]: abcd &amp;\r\n    ==== \t\r\n\r\nuse [^n]\r\n", 3,
     "[^n]: ### abcd &amp;\r\n\r\nuse [^n]\r\n", "abcd &amp;"},
    {"[r]: /unused 'Title'\n\nabcd &amp;\n====\n\n[r]: /duplicate \"Other\"\n", 0,
     "[r]: /unused 'Title'\n\nabcd &amp;\n\n[r]: /duplicate \"Other\"\n", "abcd &amp;"},
    {"[r]: /unused\nabcd &amp;\n====\n", 2,
     "[r]: /unused\nabcd &amp;\n----\n", "abcd &amp;"},
    {"## abcd \\# &amp; ###\n", 0, "abcd \\# &amp;\n", "abcd \\# &amp;"},
    {"> - ##\n", 4, "> - ####\n", ""},
    {"## ### \t\r\n", 4, "#### ### \t\r\n", ""},
    {"## abcd &amp;\n\nTAIL  \n", 0, "abcd &amp;\n\nTAIL  \n", "abcd &amp;"},
    {"**abcd** &amp;  \r\n", 2, "## **abcd** &amp;  \r\n", NULL},
    {"> - **abcd** &amp;  \r\n", 2, "> - ## **abcd** &amp;  \r\n", NULL}
};
#endif
