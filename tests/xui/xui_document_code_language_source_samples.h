#ifndef XUI_DOCUMENT_CODE_LANGUAGE_SOURCE_SAMPLES_H
#define XUI_DOCUMENT_CODE_LANGUAGE_SOURCE_SAMPLES_H
static const struct { const char* input; const char* language; const char* expected; } code_language_source_samples[] = {
    {"~~~~  c  {.numbers key=\"a&amp;b\"}   \r\nabc\r\n~~~~~ \t\r\n", "cpp",
     "~~~~  cpp  {.numbers key=\"a&amp;b\"}   \r\nabc\r\n~~~~~ \t\r\n"},
    {"before &amp;\n\n```c extra\nabc\n`````\n\n[r]: /same 'Title'\n\nafter [r]\n", "rust",
     "before &amp;\n\n```rust extra\nabc\n`````\n\n[r]: /same 'Title'\n\nafter [r]\n"},
    {"> - ~~~~c\t{.numbers}  \r\n>   abc\r\n>   ~~~~~\r\n", "cpp",
     "> - ~~~~cpp\t{.numbers}  \r\n>   abc\r\n>   ~~~~~\r\n"},
    {"- > ```c metadata\n  > abc\n  > ````\n\n  [r]: /unused\n", "js",
     "- > ```js metadata\n  > abc\n  > ````\n\n  [r]: /unused\n"},
    {"\xef\xbb\xbf~~~c extra\rabc\r~~~~", "cpp",
     "\xef\xbb\xbf~~~cpp extra\rabc\r~~~~"},
    {"```c extra\nabc", "cpp", "```cpp extra\nabc"},
    {"~~~   \nabc\n~~~\n", "cpp", "~~~   cpp\nabc\n~~~\n"},
    {"~~~ c   \nabc\n~~~\n", "", "~~~    \nabc\n~~~\n"},
    {"~~~c&amp;pp extra\nabc\n~~~~\n", "c++", "~~~c++ extra\nabc\n~~~~\n"},
    {"~~~c extra\nabc\n~~~~\n", "c&amp;pp", "~~~c&amp;amp;pp extra\nabc\n~~~~\n"},
    {"~~~c\\+\\+ extra\nabc\n~~~\n", "c\\lang", "~~~c&#92;lang extra\nabc\n~~~\n"},
    {"~~~c extra\nabc\n~~~\n", "C++标准", "~~~C++标准 extra\nabc\n~~~\n"},
    {"[^n]: ~~~~c metadata\r\n    abc\r\n    ~~~~~\r\n\r\nuse [^n]\r\n", "cpp",
     "[^n]: ~~~~cpp metadata\r\n    abc\r\n    ~~~~~\r\n\r\nuse [^n]\r\n"}
};
#endif
