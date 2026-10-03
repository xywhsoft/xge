#ifndef XUI_DOCUMENT_NESTED_PREFIX_SAMPLES_H
#define XUI_DOCUMENT_NESTED_PREFIX_SAMPLES_H
static const struct { const char* input; const char* expected; int merge; } document_nested_prefix_samples[] = {
        {"+ > abcd\n  >\n  > [unused]: /raw 'Title'\n",
            "+ > ab\n  > \n  > cd\n  > \n  >\n  > [unused]: /raw 'Title'\n", 0},
        {"12) > > abcd\r\n    > >\r\n    > > [unused]: <raw> \"Title\"\r\n",
            "12) > > ab\r\n    > > \r\n    > > cd\r\n    > > \r\n    > >\r\n    > > [unused]: <raw> \"Title\"\r\n", 0},
        {"> + > abcd\n>   >\n>   > [unused]: /raw 'Title'\n",
            "> + > ab\n>   > \n>   > cd\n>   > \n>   >\n>   > [unused]: /raw 'Title'\n", 0},
        {"- > + > abcd\n  >   >\n  >   > [unused]: /raw 'Title'\n",
            "- > + > ab\n  >   > \n  >   > cd\n  >   > \n  >   >\n  >   > [unused]: /raw 'Title'\n", 0},
        {"+ > **abcd**\n  >\n  > [unused]: /raw 'Title'\n",
            "+ > **ab**\n  > \n  > **cd**\n  > \n  >\n  > [unused]: /raw 'Title'\n", 0},
        {"before &amp; untouched\n\n-\t>\tabcd\n \t>\t\n \t>\t[unused]: /raw 'Title'\n\nafter &amp; untouched\n",
            "before &amp; untouched\n\n-\t>\tab\n \t>\t\n \t>\tcd\n \t>\t\n \t>\t\n \t>\t[unused]: /raw 'Title'\n\nafter &amp; untouched\n", 0},
        {"- > 7) [x] abcd\n  >    \n  >    [unused]: /raw 'Title'\n",
            "- > 7) [x] ab\n  >    \n  >    cd\n  >    \n  >    \n  >    [unused]: /raw 'Title'\n", 0},
        {"+ > ab\n  >\n  > cd\n  >\n  > [unused]: /raw 'Title'\n",
            "+ > abcd\n  >\n  > [unused]: /raw 'Title'\n", 1},
        {"> 12) > ab\r\n>     >\r\n>     > [unused]: /raw 'Title'\r\n>     >\r\n>     > cd\r\n",
            "> 12) > abcd\r\n>     >\r\n>     > [unused]: /raw 'Title'\r\n>     >\r\n", 1}
    };
#endif
