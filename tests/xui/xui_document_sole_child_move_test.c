#include "xui_document.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct block_case {
    const char *name, *before, *after;
    unsigned moving_kind, target_kind, target_empty;
    uint64_t index;
} block_case;
static const block_case cases[] = {
    {"paragraph-sole-quote-to-root", "> BETA &#60;\n\nrest __same__\n", "> \n\nBETA &#60;\n\nrest __same__\n", XUI_DOC_PARAGRAPH, XUI_DOC_ROOT, 0, 1},
    {"paragraph-sole-item-to-root", "+ BETA &#60;\n\nrest __same__\n", "+ \n\nBETA &#60;\n\nrest __same__\n", XUI_DOC_PARAGRAPH, XUI_DOC_ROOT, 0, 1},
    {"paragraph-sole-footnote-to-root", "anchor [^f]\n\n[^f]: BETA &#60;\n\nrest __same__\n", "anchor [^f]\n\n[^f]: \n\nBETA &#60;\n\nrest __same__\n", XUI_DOC_PARAGRAPH, XUI_DOC_ROOT, 0, 1},
    {"heading-sole-quote-to-root", "> # BETA &#60;\n\nrest __same__\n", "> \n\n# BETA &#60;\n\nrest __same__\n", XUI_DOC_HEADING, XUI_DOC_ROOT, 0, 1},
    {"heading-sole-item-to-root", "+ # BETA &#60;\n\nrest __same__\n", "+ \n\n# BETA &#60;\n\nrest __same__\n", XUI_DOC_HEADING, XUI_DOC_ROOT, 0, 1},
    {"heading-sole-footnote-to-root", "anchor [^f]\n\n[^f]: # BETA &#60;\n\nrest __same__\n", "anchor [^f]\n\n[^f]: \n\n# BETA &#60;\n\nrest __same__\n", XUI_DOC_HEADING, XUI_DOC_ROOT, 0, 1},
    {"fenced-sole-quote-to-root", "> ~~~ c extra\n> BETA &amp;\n> ~~~\n\nrest __same__\n", "> \n\n~~~ c extra\nBETA &amp;\n~~~\n\nrest __same__\n", XUI_DOC_CODE_BLOCK, XUI_DOC_ROOT, 0, 1},
    {"fenced-sole-item-to-root", "+ ~~~ c extra\n  BETA &amp;\n  ~~~\n\nrest __same__\n", "+ \n\n~~~ c extra\nBETA &amp;\n~~~\n\nrest __same__\n", XUI_DOC_CODE_BLOCK, XUI_DOC_ROOT, 0, 1},
    {"fenced-sole-footnote-to-root", "anchor [^f]\n\n[^f]: ~~~ c extra\n    BETA &amp;\n    ~~~\n\nrest __same__\n", "anchor [^f]\n\n[^f]: \n\n~~~ c extra\nBETA &amp;\n~~~\n\nrest __same__\n", XUI_DOC_CODE_BLOCK, XUI_DOC_ROOT, 0, 1},
    {"list-sole-quote-to-root", "> - BETA &#60;\n> - same __raw__\n\nrest __same__\n", "> \n\n- BETA &#60;\n- same __raw__\n\nrest __same__\n", XUI_DOC_LIST, XUI_DOC_ROOT, 0, 1},
    {"list-sole-item-to-root", "+ - BETA &#60;\n  - same __raw__\n\nrest __same__\n", "+ \n\n- BETA &#60;\n- same __raw__\n\nrest __same__\n", XUI_DOC_LIST, XUI_DOC_ROOT, 0, 1},
    {"list-sole-footnote-to-root", "anchor [^f]\n\n[^f]: - BETA &#60;\n    - same __raw__\n\nrest __same__\n", "anchor [^f]\n\n[^f]: \n\n- BETA &#60;\n- same __raw__\n\nrest __same__\n", XUI_DOC_LIST, XUI_DOC_ROOT, 0, 1},
    {"table-sole-quote-to-root", "> | BETA &#60; | n |\n> | :--- | ---: |\n> | __raw__ | &amp; |\n\nrest __same__\n", "> \n\n| BETA &#60; | n |\n| :--- | ---: |\n| __raw__ | &amp; |\n\nrest __same__\n", XUI_DOC_TABLE, XUI_DOC_ROOT, 0, 1},
    {"table-sole-item-to-root", "+ | BETA &#60; | n |\n  | :--- | ---: |\n  | __raw__ | &amp; |\n\nrest __same__\n", "+ \n\n| BETA &#60; | n |\n| :--- | ---: |\n| __raw__ | &amp; |\n\nrest __same__\n", XUI_DOC_TABLE, XUI_DOC_ROOT, 0, 1},
    {"table-sole-footnote-to-root", "anchor [^f]\n\n[^f]: | BETA &#60; | n |\n    | :--- | ---: |\n    | __raw__ | &amp; |\n\nrest __same__\n", "anchor [^f]\n\n[^f]: \n\n| BETA &#60; | n |\n| :--- | ---: |\n| __raw__ | &amp; |\n\nrest __same__\n", XUI_DOC_TABLE, XUI_DOC_ROOT, 0, 1},
    {"paragraph-sole-quote-to-quote-forward", "> BETA &#60;\n\n> target &lt;\n>\n> tail __same__\n", "> \n\n> target &lt;\n>\n> BETA &#60;\n>\n> tail __same__\n", XUI_DOC_PARAGRAPH, XUI_DOC_QUOTE, 0, 1},
    {"paragraph-sole-quote-to-quote-reverse", "> target &lt;\n>\n> tail __same__\n\n> BETA &#60;\n\n", "> target &lt;\n>\n> BETA &#60;\n>\n> tail __same__\n\n> \n\n", XUI_DOC_PARAGRAPH, XUI_DOC_QUOTE, 0, 1},
    {"paragraph-sole-quote-to-item-forward", "> BETA &#60;\n\n+ target &lt;\n\n  tail __same__\n", "> \n\n+ target &lt;\n\n  BETA &#60;\n\n  tail __same__\n", XUI_DOC_PARAGRAPH, XUI_DOC_LIST_ITEM, 0, 1},
    {"paragraph-sole-quote-to-item-reverse", "+ target &lt;\n\n  tail __same__\n\n> BETA &#60;\n\n", "+ target &lt;\n\n  BETA &#60;\n\n  tail __same__\n\n> \n\n", XUI_DOC_PARAGRAPH, XUI_DOC_LIST_ITEM, 0, 1},
    {"paragraph-sole-quote-to-footnote-forward", "anchor [^g]\n\n> BETA &#60;\n\n[^g]: target &lt;\n\n    tail __same__\n", "anchor [^g]\n\n> \n\n[^g]: target &lt;\n\n    BETA &#60;\n\n    tail __same__\n", XUI_DOC_PARAGRAPH, XUI_DOC_FOOTNOTE, 0, 1},
    {"paragraph-sole-quote-to-footnote-reverse", "anchor [^g]\n\n[^g]: target &lt;\n\n    tail __same__\n\n> BETA &#60;\n\n", "anchor [^g]\n\n[^g]: target &lt;\n\n    BETA &#60;\n\n    tail __same__\n\n> \n\n", XUI_DOC_PARAGRAPH, XUI_DOC_FOOTNOTE, 0, 1},
    {"paragraph-sole-item-to-quote-forward", "+ BETA &#60;\n\n> target &lt;\n>\n> tail __same__\n", "+ \n\n> target &lt;\n>\n> BETA &#60;\n>\n> tail __same__\n", XUI_DOC_PARAGRAPH, XUI_DOC_QUOTE, 0, 1},
    {"paragraph-sole-item-to-quote-reverse", "> target &lt;\n>\n> tail __same__\n\n+ BETA &#60;\n\n", "> target &lt;\n>\n> BETA &#60;\n>\n> tail __same__\n\n+ \n\n", XUI_DOC_PARAGRAPH, XUI_DOC_QUOTE, 0, 1},
    {"paragraph-sole-item-to-item-forward", "+ BETA &#60;\n\n+ target &lt;\n\n  tail __same__\n", "+ \n\n+ target &lt;\n\n  BETA &#60;\n\n  tail __same__\n", XUI_DOC_PARAGRAPH, XUI_DOC_LIST_ITEM, 0, 1},
    {"paragraph-sole-item-to-item-reverse", "+ target &lt;\n\n  tail __same__\n\n+ BETA &#60;\n\n", "+ target &lt;\n\n  BETA &#60;\n\n  tail __same__\n\n+ \n\n", XUI_DOC_PARAGRAPH, XUI_DOC_LIST_ITEM, 0, 1},
    {"paragraph-sole-item-to-footnote-forward", "anchor [^g]\n\n+ BETA &#60;\n\n[^g]: target &lt;\n\n    tail __same__\n", "anchor [^g]\n\n+ \n\n[^g]: target &lt;\n\n    BETA &#60;\n\n    tail __same__\n", XUI_DOC_PARAGRAPH, XUI_DOC_FOOTNOTE, 0, 1},
    {"paragraph-sole-item-to-footnote-reverse", "anchor [^g]\n\n[^g]: target &lt;\n\n    tail __same__\n\n+ BETA &#60;\n\n", "anchor [^g]\n\n[^g]: target &lt;\n\n    BETA &#60;\n\n    tail __same__\n\n+ \n\n", XUI_DOC_PARAGRAPH, XUI_DOC_FOOTNOTE, 0, 1},
    {"paragraph-sole-footnote-to-quote-forward", "anchor [^f]\n\n[^f]: BETA &#60;\n\n> target &lt;\n>\n> tail __same__\n", "anchor [^f]\n\n[^f]: \n\n> target &lt;\n>\n> BETA &#60;\n>\n> tail __same__\n", XUI_DOC_PARAGRAPH, XUI_DOC_QUOTE, 0, 1},
    {"paragraph-sole-footnote-to-quote-reverse", "anchor [^f]\n\n> target &lt;\n>\n> tail __same__\n\n[^f]: BETA &#60;\n\n", "anchor [^f]\n\n> target &lt;\n>\n> BETA &#60;\n>\n> tail __same__\n\n[^f]: \n\n", XUI_DOC_PARAGRAPH, XUI_DOC_QUOTE, 0, 1},
    {"paragraph-sole-footnote-to-item-forward", "anchor [^f]\n\n[^f]: BETA &#60;\n\n+ target &lt;\n\n  tail __same__\n", "anchor [^f]\n\n[^f]: \n\n+ target &lt;\n\n  BETA &#60;\n\n  tail __same__\n", XUI_DOC_PARAGRAPH, XUI_DOC_LIST_ITEM, 0, 1},
    {"paragraph-sole-footnote-to-item-reverse", "anchor [^f]\n\n+ target &lt;\n\n  tail __same__\n\n[^f]: BETA &#60;\n\n", "anchor [^f]\n\n+ target &lt;\n\n  BETA &#60;\n\n  tail __same__\n\n[^f]: \n\n", XUI_DOC_PARAGRAPH, XUI_DOC_LIST_ITEM, 0, 1},
    {"paragraph-sole-footnote-to-footnote-forward", "anchor [^f] [^g]\n\n[^f]: BETA &#60;\n\n[^g]: target &lt;\n\n    tail __same__\n", "anchor [^f] [^g]\n\n[^f]: \n\n[^g]: target &lt;\n\n    BETA &#60;\n\n    tail __same__\n", XUI_DOC_PARAGRAPH, XUI_DOC_FOOTNOTE, 0, 1},
    {"paragraph-sole-footnote-to-footnote-reverse", "anchor [^f] [^g]\n\n[^g]: target &lt;\n\n    tail __same__\n\n[^f]: BETA &#60;\n\n", "anchor [^f] [^g]\n\n[^g]: target &lt;\n\n    BETA &#60;\n\n    tail __same__\n\n[^f]: \n\n", XUI_DOC_PARAGRAPH, XUI_DOC_FOOTNOTE, 0, 1},
    {"heading-sole-quote-to-quote-forward", "> # BETA &#60;\n\n> target &lt;\n>\n> tail __same__\n", "> \n\n> target &lt;\n>\n> # BETA &#60;\n>\n> tail __same__\n", XUI_DOC_HEADING, XUI_DOC_QUOTE, 0, 1},
    {"heading-sole-quote-to-quote-reverse", "> target &lt;\n>\n> tail __same__\n\n> # BETA &#60;\n\n", "> target &lt;\n>\n> # BETA &#60;\n>\n> tail __same__\n\n> \n\n", XUI_DOC_HEADING, XUI_DOC_QUOTE, 0, 1},
    {"heading-sole-quote-to-item-forward", "> # BETA &#60;\n\n+ target &lt;\n\n  tail __same__\n", "> \n\n+ target &lt;\n\n  # BETA &#60;\n\n  tail __same__\n", XUI_DOC_HEADING, XUI_DOC_LIST_ITEM, 0, 1},
    {"heading-sole-quote-to-item-reverse", "+ target &lt;\n\n  tail __same__\n\n> # BETA &#60;\n\n", "+ target &lt;\n\n  # BETA &#60;\n\n  tail __same__\n\n> \n\n", XUI_DOC_HEADING, XUI_DOC_LIST_ITEM, 0, 1},
    {"heading-sole-quote-to-footnote-forward", "anchor [^g]\n\n> # BETA &#60;\n\n[^g]: target &lt;\n\n    tail __same__\n", "anchor [^g]\n\n> \n\n[^g]: target &lt;\n\n    # BETA &#60;\n\n    tail __same__\n", XUI_DOC_HEADING, XUI_DOC_FOOTNOTE, 0, 1},
    {"heading-sole-quote-to-footnote-reverse", "anchor [^g]\n\n[^g]: target &lt;\n\n    tail __same__\n\n> # BETA &#60;\n\n", "anchor [^g]\n\n[^g]: target &lt;\n\n    # BETA &#60;\n\n    tail __same__\n\n> \n\n", XUI_DOC_HEADING, XUI_DOC_FOOTNOTE, 0, 1},
    {"heading-sole-item-to-quote-forward", "+ # BETA &#60;\n\n> target &lt;\n>\n> tail __same__\n", "+ \n\n> target &lt;\n>\n> # BETA &#60;\n>\n> tail __same__\n", XUI_DOC_HEADING, XUI_DOC_QUOTE, 0, 1},
    {"heading-sole-item-to-quote-reverse", "> target &lt;\n>\n> tail __same__\n\n+ # BETA &#60;\n\n", "> target &lt;\n>\n> # BETA &#60;\n>\n> tail __same__\n\n+ \n\n", XUI_DOC_HEADING, XUI_DOC_QUOTE, 0, 1},
    {"heading-sole-item-to-item-forward", "+ # BETA &#60;\n\n+ target &lt;\n\n  tail __same__\n", "+ \n\n+ target &lt;\n\n  # BETA &#60;\n\n  tail __same__\n", XUI_DOC_HEADING, XUI_DOC_LIST_ITEM, 0, 1},
    {"heading-sole-item-to-item-reverse", "+ target &lt;\n\n  tail __same__\n\n+ # BETA &#60;\n\n", "+ target &lt;\n\n  # BETA &#60;\n\n  tail __same__\n\n+ \n\n", XUI_DOC_HEADING, XUI_DOC_LIST_ITEM, 0, 1},
    {"heading-sole-item-to-footnote-forward", "anchor [^g]\n\n+ # BETA &#60;\n\n[^g]: target &lt;\n\n    tail __same__\n", "anchor [^g]\n\n+ \n\n[^g]: target &lt;\n\n    # BETA &#60;\n\n    tail __same__\n", XUI_DOC_HEADING, XUI_DOC_FOOTNOTE, 0, 1},
    {"heading-sole-item-to-footnote-reverse", "anchor [^g]\n\n[^g]: target &lt;\n\n    tail __same__\n\n+ # BETA &#60;\n\n", "anchor [^g]\n\n[^g]: target &lt;\n\n    # BETA &#60;\n\n    tail __same__\n\n+ \n\n", XUI_DOC_HEADING, XUI_DOC_FOOTNOTE, 0, 1},
    {"heading-sole-footnote-to-quote-forward", "anchor [^f]\n\n[^f]: # BETA &#60;\n\n> target &lt;\n>\n> tail __same__\n", "anchor [^f]\n\n[^f]: \n\n> target &lt;\n>\n> # BETA &#60;\n>\n> tail __same__\n", XUI_DOC_HEADING, XUI_DOC_QUOTE, 0, 1},
    {"heading-sole-footnote-to-quote-reverse", "anchor [^f]\n\n> target &lt;\n>\n> tail __same__\n\n[^f]: # BETA &#60;\n\n", "anchor [^f]\n\n> target &lt;\n>\n> # BETA &#60;\n>\n> tail __same__\n\n[^f]: \n\n", XUI_DOC_HEADING, XUI_DOC_QUOTE, 0, 1},
    {"heading-sole-footnote-to-item-forward", "anchor [^f]\n\n[^f]: # BETA &#60;\n\n+ target &lt;\n\n  tail __same__\n", "anchor [^f]\n\n[^f]: \n\n+ target &lt;\n\n  # BETA &#60;\n\n  tail __same__\n", XUI_DOC_HEADING, XUI_DOC_LIST_ITEM, 0, 1},
    {"heading-sole-footnote-to-item-reverse", "anchor [^f]\n\n+ target &lt;\n\n  tail __same__\n\n[^f]: # BETA &#60;\n\n", "anchor [^f]\n\n+ target &lt;\n\n  # BETA &#60;\n\n  tail __same__\n\n[^f]: \n\n", XUI_DOC_HEADING, XUI_DOC_LIST_ITEM, 0, 1},
    {"heading-sole-footnote-to-footnote-forward", "anchor [^f] [^g]\n\n[^f]: # BETA &#60;\n\n[^g]: target &lt;\n\n    tail __same__\n", "anchor [^f] [^g]\n\n[^f]: \n\n[^g]: target &lt;\n\n    # BETA &#60;\n\n    tail __same__\n", XUI_DOC_HEADING, XUI_DOC_FOOTNOTE, 0, 1},
    {"heading-sole-footnote-to-footnote-reverse", "anchor [^f] [^g]\n\n[^g]: target &lt;\n\n    tail __same__\n\n[^f]: # BETA &#60;\n\n", "anchor [^f] [^g]\n\n[^g]: target &lt;\n\n    # BETA &#60;\n\n    tail __same__\n\n[^f]: \n\n", XUI_DOC_HEADING, XUI_DOC_FOOTNOTE, 0, 1},
    {"fenced-sole-quote-to-quote-forward", "> ~~~ c extra\n> BETA &amp;\n> ~~~\n\n> target &lt;\n>\n> tail __same__\n", "> \n\n> target &lt;\n>\n> ~~~ c extra\n> BETA &amp;\n> ~~~\n>\n> tail __same__\n", XUI_DOC_CODE_BLOCK, XUI_DOC_QUOTE, 0, 1},
    {"fenced-sole-quote-to-quote-reverse", "> target &lt;\n>\n> tail __same__\n\n> ~~~ c extra\n> BETA &amp;\n> ~~~\n\n", "> target &lt;\n>\n> ~~~ c extra\n> BETA &amp;\n> ~~~\n>\n> tail __same__\n\n> \n\n", XUI_DOC_CODE_BLOCK, XUI_DOC_QUOTE, 0, 1},
    {"fenced-sole-quote-to-item-forward", "> ~~~ c extra\n> BETA &amp;\n> ~~~\n\n+ target &lt;\n\n  tail __same__\n", "> \n\n+ target &lt;\n\n  ~~~ c extra\n  BETA &amp;\n  ~~~\n\n  tail __same__\n", XUI_DOC_CODE_BLOCK, XUI_DOC_LIST_ITEM, 0, 1},
    {"fenced-sole-quote-to-item-reverse", "+ target &lt;\n\n  tail __same__\n\n> ~~~ c extra\n> BETA &amp;\n> ~~~\n\n", "+ target &lt;\n\n  ~~~ c extra\n  BETA &amp;\n  ~~~\n\n  tail __same__\n\n> \n\n", XUI_DOC_CODE_BLOCK, XUI_DOC_LIST_ITEM, 0, 1},
    {"fenced-sole-quote-to-footnote-forward", "anchor [^g]\n\n> ~~~ c extra\n> BETA &amp;\n> ~~~\n\n[^g]: target &lt;\n\n    tail __same__\n", "anchor [^g]\n\n> \n\n[^g]: target &lt;\n\n    ~~~ c extra\n    BETA &amp;\n    ~~~\n\n    tail __same__\n", XUI_DOC_CODE_BLOCK, XUI_DOC_FOOTNOTE, 0, 1},
    {"fenced-sole-quote-to-footnote-reverse", "anchor [^g]\n\n[^g]: target &lt;\n\n    tail __same__\n\n> ~~~ c extra\n> BETA &amp;\n> ~~~\n\n", "anchor [^g]\n\n[^g]: target &lt;\n\n    ~~~ c extra\n    BETA &amp;\n    ~~~\n\n    tail __same__\n\n> \n\n", XUI_DOC_CODE_BLOCK, XUI_DOC_FOOTNOTE, 0, 1},
    {"fenced-sole-item-to-quote-forward", "+ ~~~ c extra\n  BETA &amp;\n  ~~~\n\n> target &lt;\n>\n> tail __same__\n", "+ \n\n> target &lt;\n>\n> ~~~ c extra\n> BETA &amp;\n> ~~~\n>\n> tail __same__\n", XUI_DOC_CODE_BLOCK, XUI_DOC_QUOTE, 0, 1},
    {"fenced-sole-item-to-quote-reverse", "> target &lt;\n>\n> tail __same__\n\n+ ~~~ c extra\n  BETA &amp;\n  ~~~\n\n", "> target &lt;\n>\n> ~~~ c extra\n> BETA &amp;\n> ~~~\n>\n> tail __same__\n\n+ \n\n", XUI_DOC_CODE_BLOCK, XUI_DOC_QUOTE, 0, 1},
    {"fenced-sole-item-to-item-forward", "+ ~~~ c extra\n  BETA &amp;\n  ~~~\n\n+ target &lt;\n\n  tail __same__\n", "+ \n\n+ target &lt;\n\n  ~~~ c extra\n  BETA &amp;\n  ~~~\n\n  tail __same__\n", XUI_DOC_CODE_BLOCK, XUI_DOC_LIST_ITEM, 0, 1},
    {"fenced-sole-item-to-item-reverse", "+ target &lt;\n\n  tail __same__\n\n+ ~~~ c extra\n  BETA &amp;\n  ~~~\n\n", "+ target &lt;\n\n  ~~~ c extra\n  BETA &amp;\n  ~~~\n\n  tail __same__\n\n+ \n\n", XUI_DOC_CODE_BLOCK, XUI_DOC_LIST_ITEM, 0, 1},
    {"fenced-sole-item-to-footnote-forward", "anchor [^g]\n\n+ ~~~ c extra\n  BETA &amp;\n  ~~~\n\n[^g]: target &lt;\n\n    tail __same__\n", "anchor [^g]\n\n+ \n\n[^g]: target &lt;\n\n    ~~~ c extra\n    BETA &amp;\n    ~~~\n\n    tail __same__\n", XUI_DOC_CODE_BLOCK, XUI_DOC_FOOTNOTE, 0, 1},
    {"fenced-sole-item-to-footnote-reverse", "anchor [^g]\n\n[^g]: target &lt;\n\n    tail __same__\n\n+ ~~~ c extra\n  BETA &amp;\n  ~~~\n\n", "anchor [^g]\n\n[^g]: target &lt;\n\n    ~~~ c extra\n    BETA &amp;\n    ~~~\n\n    tail __same__\n\n+ \n\n", XUI_DOC_CODE_BLOCK, XUI_DOC_FOOTNOTE, 0, 1},
    {"fenced-sole-footnote-to-quote-forward", "anchor [^f]\n\n[^f]: ~~~ c extra\n    BETA &amp;\n    ~~~\n\n> target &lt;\n>\n> tail __same__\n", "anchor [^f]\n\n[^f]: \n\n> target &lt;\n>\n> ~~~ c extra\n> BETA &amp;\n> ~~~\n>\n> tail __same__\n", XUI_DOC_CODE_BLOCK, XUI_DOC_QUOTE, 0, 1},
    {"fenced-sole-footnote-to-quote-reverse", "anchor [^f]\n\n> target &lt;\n>\n> tail __same__\n\n[^f]: ~~~ c extra\n    BETA &amp;\n    ~~~\n\n", "anchor [^f]\n\n> target &lt;\n>\n> ~~~ c extra\n> BETA &amp;\n> ~~~\n>\n> tail __same__\n\n[^f]: \n\n", XUI_DOC_CODE_BLOCK, XUI_DOC_QUOTE, 0, 1},
    {"fenced-sole-footnote-to-item-forward", "anchor [^f]\n\n[^f]: ~~~ c extra\n    BETA &amp;\n    ~~~\n\n+ target &lt;\n\n  tail __same__\n", "anchor [^f]\n\n[^f]: \n\n+ target &lt;\n\n  ~~~ c extra\n  BETA &amp;\n  ~~~\n\n  tail __same__\n", XUI_DOC_CODE_BLOCK, XUI_DOC_LIST_ITEM, 0, 1},
    {"fenced-sole-footnote-to-item-reverse", "anchor [^f]\n\n+ target &lt;\n\n  tail __same__\n\n[^f]: ~~~ c extra\n    BETA &amp;\n    ~~~\n\n", "anchor [^f]\n\n+ target &lt;\n\n  ~~~ c extra\n  BETA &amp;\n  ~~~\n\n  tail __same__\n\n[^f]: \n\n", XUI_DOC_CODE_BLOCK, XUI_DOC_LIST_ITEM, 0, 1},
    {"fenced-sole-footnote-to-footnote-forward", "anchor [^f] [^g]\n\n[^f]: ~~~ c extra\n    BETA &amp;\n    ~~~\n\n[^g]: target &lt;\n\n    tail __same__\n", "anchor [^f] [^g]\n\n[^f]: \n\n[^g]: target &lt;\n\n    ~~~ c extra\n    BETA &amp;\n    ~~~\n\n    tail __same__\n", XUI_DOC_CODE_BLOCK, XUI_DOC_FOOTNOTE, 0, 1},
    {"fenced-sole-footnote-to-footnote-reverse", "anchor [^f] [^g]\n\n[^g]: target &lt;\n\n    tail __same__\n\n[^f]: ~~~ c extra\n    BETA &amp;\n    ~~~\n\n", "anchor [^f] [^g]\n\n[^g]: target &lt;\n\n    ~~~ c extra\n    BETA &amp;\n    ~~~\n\n    tail __same__\n\n[^f]: \n\n", XUI_DOC_CODE_BLOCK, XUI_DOC_FOOTNOTE, 0, 1},
    {"list-sole-quote-to-quote-forward", "> - BETA &#60;\n> - same __raw__\n\n> target &lt;\n>\n> tail __same__\n", "> \n\n> target &lt;\n>\n> - BETA &#60;\n> - same __raw__\n>\n> tail __same__\n", XUI_DOC_LIST, XUI_DOC_QUOTE, 0, 1},
    {"list-sole-quote-to-quote-reverse", "> target &lt;\n>\n> tail __same__\n\n> - BETA &#60;\n> - same __raw__\n\n", "> target &lt;\n>\n> - BETA &#60;\n> - same __raw__\n>\n> tail __same__\n\n> \n\n", XUI_DOC_LIST, XUI_DOC_QUOTE, 0, 1},
    {"list-sole-quote-to-item-forward", "> - BETA &#60;\n> - same __raw__\n\n+ target &lt;\n\n  tail __same__\n", "> \n\n+ target &lt;\n\n  - BETA &#60;\n  - same __raw__\n\n  tail __same__\n", XUI_DOC_LIST, XUI_DOC_LIST_ITEM, 0, 1},
    {"list-sole-quote-to-item-reverse", "+ target &lt;\n\n  tail __same__\n\n> - BETA &#60;\n> - same __raw__\n\n", "+ target &lt;\n\n  - BETA &#60;\n  - same __raw__\n\n  tail __same__\n\n> \n\n", XUI_DOC_LIST, XUI_DOC_LIST_ITEM, 0, 1},
    {"list-sole-quote-to-footnote-forward", "anchor [^g]\n\n> - BETA &#60;\n> - same __raw__\n\n[^g]: target &lt;\n\n    tail __same__\n", "anchor [^g]\n\n> \n\n[^g]: target &lt;\n\n    - BETA &#60;\n    - same __raw__\n\n    tail __same__\n", XUI_DOC_LIST, XUI_DOC_FOOTNOTE, 0, 1},
    {"list-sole-quote-to-footnote-reverse", "anchor [^g]\n\n[^g]: target &lt;\n\n    tail __same__\n\n> - BETA &#60;\n> - same __raw__\n\n", "anchor [^g]\n\n[^g]: target &lt;\n\n    - BETA &#60;\n    - same __raw__\n\n    tail __same__\n\n> \n\n", XUI_DOC_LIST, XUI_DOC_FOOTNOTE, 0, 1},
    {"list-sole-item-to-quote-forward", "+ - BETA &#60;\n  - same __raw__\n\n> target &lt;\n>\n> tail __same__\n", "+ \n\n> target &lt;\n>\n> - BETA &#60;\n> - same __raw__\n>\n> tail __same__\n", XUI_DOC_LIST, XUI_DOC_QUOTE, 0, 1},
    {"list-sole-item-to-quote-reverse", "> target &lt;\n>\n> tail __same__\n\n+ - BETA &#60;\n  - same __raw__\n\n", "> target &lt;\n>\n> - BETA &#60;\n> - same __raw__\n>\n> tail __same__\n\n+ \n\n", XUI_DOC_LIST, XUI_DOC_QUOTE, 0, 1},
    {"list-sole-item-to-item-forward", "+ - BETA &#60;\n  - same __raw__\n\n+ target &lt;\n\n  tail __same__\n", "+ \n\n+ target &lt;\n\n  - BETA &#60;\n  - same __raw__\n\n  tail __same__\n", XUI_DOC_LIST, XUI_DOC_LIST_ITEM, 0, 1},
    {"list-sole-item-to-item-reverse", "+ target &lt;\n\n  tail __same__\n\n+ - BETA &#60;\n  - same __raw__\n\n", "+ target &lt;\n\n  - BETA &#60;\n  - same __raw__\n\n  tail __same__\n\n+ \n\n", XUI_DOC_LIST, XUI_DOC_LIST_ITEM, 0, 1},
    {"list-sole-item-to-footnote-forward", "anchor [^g]\n\n+ - BETA &#60;\n  - same __raw__\n\n[^g]: target &lt;\n\n    tail __same__\n", "anchor [^g]\n\n+ \n\n[^g]: target &lt;\n\n    - BETA &#60;\n    - same __raw__\n\n    tail __same__\n", XUI_DOC_LIST, XUI_DOC_FOOTNOTE, 0, 1},
    {"list-sole-item-to-footnote-reverse", "anchor [^g]\n\n[^g]: target &lt;\n\n    tail __same__\n\n+ - BETA &#60;\n  - same __raw__\n\n", "anchor [^g]\n\n[^g]: target &lt;\n\n    - BETA &#60;\n    - same __raw__\n\n    tail __same__\n\n+ \n\n", XUI_DOC_LIST, XUI_DOC_FOOTNOTE, 0, 1},
    {"list-sole-footnote-to-quote-forward", "anchor [^f]\n\n[^f]: - BETA &#60;\n    - same __raw__\n\n> target &lt;\n>\n> tail __same__\n", "anchor [^f]\n\n[^f]: \n\n> target &lt;\n>\n> - BETA &#60;\n> - same __raw__\n>\n> tail __same__\n", XUI_DOC_LIST, XUI_DOC_QUOTE, 0, 1},
    {"list-sole-footnote-to-quote-reverse", "anchor [^f]\n\n> target &lt;\n>\n> tail __same__\n\n[^f]: - BETA &#60;\n    - same __raw__\n\n", "anchor [^f]\n\n> target &lt;\n>\n> - BETA &#60;\n> - same __raw__\n>\n> tail __same__\n\n[^f]: \n\n", XUI_DOC_LIST, XUI_DOC_QUOTE, 0, 1},
    {"list-sole-footnote-to-item-forward", "anchor [^f]\n\n[^f]: - BETA &#60;\n    - same __raw__\n\n+ target &lt;\n\n  tail __same__\n", "anchor [^f]\n\n[^f]: \n\n+ target &lt;\n\n  - BETA &#60;\n  - same __raw__\n\n  tail __same__\n", XUI_DOC_LIST, XUI_DOC_LIST_ITEM, 0, 1},
    {"list-sole-footnote-to-item-reverse", "anchor [^f]\n\n+ target &lt;\n\n  tail __same__\n\n[^f]: - BETA &#60;\n    - same __raw__\n\n", "anchor [^f]\n\n+ target &lt;\n\n  - BETA &#60;\n  - same __raw__\n\n  tail __same__\n\n[^f]: \n\n", XUI_DOC_LIST, XUI_DOC_LIST_ITEM, 0, 1},
    {"list-sole-footnote-to-footnote-forward", "anchor [^f] [^g]\n\n[^f]: - BETA &#60;\n    - same __raw__\n\n[^g]: target &lt;\n\n    tail __same__\n", "anchor [^f] [^g]\n\n[^f]: \n\n[^g]: target &lt;\n\n    - BETA &#60;\n    - same __raw__\n\n    tail __same__\n", XUI_DOC_LIST, XUI_DOC_FOOTNOTE, 0, 1},
    {"list-sole-footnote-to-footnote-reverse", "anchor [^f] [^g]\n\n[^g]: target &lt;\n\n    tail __same__\n\n[^f]: - BETA &#60;\n    - same __raw__\n\n", "anchor [^f] [^g]\n\n[^g]: target &lt;\n\n    - BETA &#60;\n    - same __raw__\n\n    tail __same__\n\n[^f]: \n\n", XUI_DOC_LIST, XUI_DOC_FOOTNOTE, 0, 1},
    {"table-sole-quote-to-quote-forward", "> | BETA &#60; | n |\n> | :--- | ---: |\n> | __raw__ | &amp; |\n\n> target &lt;\n>\n> tail __same__\n", "> \n\n> target &lt;\n>\n> | BETA &#60; | n |\n> | :--- | ---: |\n> | __raw__ | &amp; |\n>\n> tail __same__\n", XUI_DOC_TABLE, XUI_DOC_QUOTE, 0, 1},
    {"table-sole-quote-to-quote-reverse", "> target &lt;\n>\n> tail __same__\n\n> | BETA &#60; | n |\n> | :--- | ---: |\n> | __raw__ | &amp; |\n\n", "> target &lt;\n>\n> | BETA &#60; | n |\n> | :--- | ---: |\n> | __raw__ | &amp; |\n>\n> tail __same__\n\n> \n\n", XUI_DOC_TABLE, XUI_DOC_QUOTE, 0, 1},
    {"table-sole-quote-to-item-forward", "> | BETA &#60; | n |\n> | :--- | ---: |\n> | __raw__ | &amp; |\n\n+ target &lt;\n\n  tail __same__\n", "> \n\n+ target &lt;\n\n  | BETA &#60; | n |\n  | :--- | ---: |\n  | __raw__ | &amp; |\n\n  tail __same__\n", XUI_DOC_TABLE, XUI_DOC_LIST_ITEM, 0, 1},
    {"table-sole-quote-to-item-reverse", "+ target &lt;\n\n  tail __same__\n\n> | BETA &#60; | n |\n> | :--- | ---: |\n> | __raw__ | &amp; |\n\n", "+ target &lt;\n\n  | BETA &#60; | n |\n  | :--- | ---: |\n  | __raw__ | &amp; |\n\n  tail __same__\n\n> \n\n", XUI_DOC_TABLE, XUI_DOC_LIST_ITEM, 0, 1},
    {"table-sole-quote-to-footnote-forward", "anchor [^g]\n\n> | BETA &#60; | n |\n> | :--- | ---: |\n> | __raw__ | &amp; |\n\n[^g]: target &lt;\n\n    tail __same__\n", "anchor [^g]\n\n> \n\n[^g]: target &lt;\n\n    | BETA &#60; | n |\n    | :--- | ---: |\n    | __raw__ | &amp; |\n\n    tail __same__\n", XUI_DOC_TABLE, XUI_DOC_FOOTNOTE, 0, 1},
    {"table-sole-quote-to-footnote-reverse", "anchor [^g]\n\n[^g]: target &lt;\n\n    tail __same__\n\n> | BETA &#60; | n |\n> | :--- | ---: |\n> | __raw__ | &amp; |\n\n", "anchor [^g]\n\n[^g]: target &lt;\n\n    | BETA &#60; | n |\n    | :--- | ---: |\n    | __raw__ | &amp; |\n\n    tail __same__\n\n> \n\n", XUI_DOC_TABLE, XUI_DOC_FOOTNOTE, 0, 1},
    {"table-sole-item-to-quote-forward", "+ | BETA &#60; | n |\n  | :--- | ---: |\n  | __raw__ | &amp; |\n\n> target &lt;\n>\n> tail __same__\n", "+ \n\n> target &lt;\n>\n> | BETA &#60; | n |\n> | :--- | ---: |\n> | __raw__ | &amp; |\n>\n> tail __same__\n", XUI_DOC_TABLE, XUI_DOC_QUOTE, 0, 1},
    {"table-sole-item-to-quote-reverse", "> target &lt;\n>\n> tail __same__\n\n+ | BETA &#60; | n |\n  | :--- | ---: |\n  | __raw__ | &amp; |\n\n", "> target &lt;\n>\n> | BETA &#60; | n |\n> | :--- | ---: |\n> | __raw__ | &amp; |\n>\n> tail __same__\n\n+ \n\n", XUI_DOC_TABLE, XUI_DOC_QUOTE, 0, 1},
    {"table-sole-item-to-item-forward", "+ | BETA &#60; | n |\n  | :--- | ---: |\n  | __raw__ | &amp; |\n\n+ target &lt;\n\n  tail __same__\n", "+ \n\n+ target &lt;\n\n  | BETA &#60; | n |\n  | :--- | ---: |\n  | __raw__ | &amp; |\n\n  tail __same__\n", XUI_DOC_TABLE, XUI_DOC_LIST_ITEM, 0, 1},
    {"table-sole-item-to-item-reverse", "+ target &lt;\n\n  tail __same__\n\n+ | BETA &#60; | n |\n  | :--- | ---: |\n  | __raw__ | &amp; |\n\n", "+ target &lt;\n\n  | BETA &#60; | n |\n  | :--- | ---: |\n  | __raw__ | &amp; |\n\n  tail __same__\n\n+ \n\n", XUI_DOC_TABLE, XUI_DOC_LIST_ITEM, 0, 1},
    {"table-sole-item-to-footnote-forward", "anchor [^g]\n\n+ | BETA &#60; | n |\n  | :--- | ---: |\n  | __raw__ | &amp; |\n\n[^g]: target &lt;\n\n    tail __same__\n", "anchor [^g]\n\n+ \n\n[^g]: target &lt;\n\n    | BETA &#60; | n |\n    | :--- | ---: |\n    | __raw__ | &amp; |\n\n    tail __same__\n", XUI_DOC_TABLE, XUI_DOC_FOOTNOTE, 0, 1},
    {"table-sole-item-to-footnote-reverse", "anchor [^g]\n\n[^g]: target &lt;\n\n    tail __same__\n\n+ | BETA &#60; | n |\n  | :--- | ---: |\n  | __raw__ | &amp; |\n\n", "anchor [^g]\n\n[^g]: target &lt;\n\n    | BETA &#60; | n |\n    | :--- | ---: |\n    | __raw__ | &amp; |\n\n    tail __same__\n\n+ \n\n", XUI_DOC_TABLE, XUI_DOC_FOOTNOTE, 0, 1},
    {"table-sole-footnote-to-quote-forward", "anchor [^f]\n\n[^f]: | BETA &#60; | n |\n    | :--- | ---: |\n    | __raw__ | &amp; |\n\n> target &lt;\n>\n> tail __same__\n", "anchor [^f]\n\n[^f]: \n\n> target &lt;\n>\n> | BETA &#60; | n |\n> | :--- | ---: |\n> | __raw__ | &amp; |\n>\n> tail __same__\n", XUI_DOC_TABLE, XUI_DOC_QUOTE, 0, 1},
    {"table-sole-footnote-to-quote-reverse", "anchor [^f]\n\n> target &lt;\n>\n> tail __same__\n\n[^f]: | BETA &#60; | n |\n    | :--- | ---: |\n    | __raw__ | &amp; |\n\n", "anchor [^f]\n\n> target &lt;\n>\n> | BETA &#60; | n |\n> | :--- | ---: |\n> | __raw__ | &amp; |\n>\n> tail __same__\n\n[^f]: \n\n", XUI_DOC_TABLE, XUI_DOC_QUOTE, 0, 1},
    {"table-sole-footnote-to-item-forward", "anchor [^f]\n\n[^f]: | BETA &#60; | n |\n    | :--- | ---: |\n    | __raw__ | &amp; |\n\n+ target &lt;\n\n  tail __same__\n", "anchor [^f]\n\n[^f]: \n\n+ target &lt;\n\n  | BETA &#60; | n |\n  | :--- | ---: |\n  | __raw__ | &amp; |\n\n  tail __same__\n", XUI_DOC_TABLE, XUI_DOC_LIST_ITEM, 0, 1},
    {"table-sole-footnote-to-item-reverse", "anchor [^f]\n\n+ target &lt;\n\n  tail __same__\n\n[^f]: | BETA &#60; | n |\n    | :--- | ---: |\n    | __raw__ | &amp; |\n\n", "anchor [^f]\n\n+ target &lt;\n\n  | BETA &#60; | n |\n  | :--- | ---: |\n  | __raw__ | &amp; |\n\n  tail __same__\n\n[^f]: \n\n", XUI_DOC_TABLE, XUI_DOC_LIST_ITEM, 0, 1},
    {"table-sole-footnote-to-footnote-forward", "anchor [^f] [^g]\n\n[^f]: | BETA &#60; | n |\n    | :--- | ---: |\n    | __raw__ | &amp; |\n\n[^g]: target &lt;\n\n    tail __same__\n", "anchor [^f] [^g]\n\n[^f]: \n\n[^g]: target &lt;\n\n    | BETA &#60; | n |\n    | :--- | ---: |\n    | __raw__ | &amp; |\n\n    tail __same__\n", XUI_DOC_TABLE, XUI_DOC_FOOTNOTE, 0, 1},
    {"table-sole-footnote-to-footnote-reverse", "anchor [^f] [^g]\n\n[^g]: target &lt;\n\n    tail __same__\n\n[^f]: | BETA &#60; | n |\n    | :--- | ---: |\n    | __raw__ | &amp; |\n\n", "anchor [^f] [^g]\n\n[^g]: target &lt;\n\n    | BETA &#60; | n |\n    | :--- | ---: |\n    | __raw__ | &amp; |\n\n    tail __same__\n\n[^f]: \n\n", XUI_DOC_TABLE, XUI_DOC_FOOTNOTE, 0, 1}
};

static int open_document(const char* source, xui_document* out)
{
    xui_doc_desc_t desc = {0}; desc.iSize = sizeof(desc);
    desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
    int result = xuiDocumentCreate(&desc, out);
    return result == XUI_OK ? xuiDocumentLoadMarkdown(*out, source, strlen(source)) : result;
}
static int source_equal(xui_document_snapshot s, const char* expected)
{
    char raw[4096]; uint64_t bytes = 0;
    return xuiDocumentSnapshotCopySource(s, raw, sizeof(raw), &bytes) == XUI_OK &&
        bytes == strlen(expected) && !memcmp(raw, expected, (size_t)bytes);
}
static int find_kind(xui_document_snapshot s, uint64_t node, unsigned kind, uint64_t* out)
{
    xui_doc_node_info_t n = {0}; n.iSize = sizeof(n);
    if (xuiDocumentSnapshotGetNode(s, node, &n) != XUI_OK) return -1;
    if (n.iKind == kind) { *out = node; return XUI_OK; }
    for (uint64_t i = 0; i < n.iChildCount; i++) {
        uint64_t child;
        if (xuiDocumentSnapshotGetChild(s, node, i, &child) != XUI_OK) return -1;
        if (find_kind(s, child, kind, out) == XUI_OK) return XUI_OK;
    }
    return -1;
}
static int moving_node(xui_document_snapshot s, const char* source, unsigned kind, uint64_t* out)
{
    const char* found = strstr(source, "BETA"); xui_doc_position_t at; int quality;
    xui_doc_node_info_t n = {0}; n.iSize = sizeof(n);
    if (!found || xuiDocumentSourceToPosition(s, (uint64_t)(found - source), &at, &quality) != XUI_OK) return -1;
    for (unsigned depth = 0; depth < 128; depth++) {
        if (xuiDocumentSnapshotGetNode(s, at.iNodeId, &n) != XUI_OK) return -1;
        if (n.iKind == kind) { *out = n.iId; return XUI_OK; }
        if (!n.iParentId) break;
        at.iNodeId = n.iParentId;
    }
    return -1;
}
static int target_node(xui_document_snapshot snapshot, const char* source, unsigned kind, uint64_t* out)
{
    if (kind == XUI_DOC_ROOT) { *out = 1; return XUI_OK; }
    const char* found = strstr(source, "target");
    xui_doc_position_t at; int quality;
    xui_doc_node_info_t node = {0}; node.iSize = sizeof(node);
    if (!found || xuiDocumentSourceToPosition(snapshot, (uint64_t)(found - source), &at, &quality) != XUI_OK) return -1;
    for (unsigned depth = 0; depth < 128; depth++) {
        if (xuiDocumentSnapshotGetNode(snapshot, at.iNodeId, &node) != XUI_OK) return -1;
        if (node.iKind == kind) { *out = node.iId; return XUI_OK; }
        if (!node.iParentId) break;
        at.iNodeId = node.iParentId;
    }
    return -1;
}
static int all_ids(xui_document_snapshot old, xui_document_snapshot now,
    uint64_t node, uint64_t moving, uint64_t new_parent)
{
    xui_doc_node_info_t a = {0}, b = {0}; a.iSize = b.iSize = sizeof(a);
    if (xuiDocumentSnapshotGetNode(old, node, &a) != XUI_OK || xuiDocumentSnapshotGetNode(now, node, &b) != XUI_OK ||
        a.iKind != b.iKind || a.iTextBytes != b.iTextBytes || b.iParentId != (node == moving ? new_parent : a.iParentId)) return 0;
    for (uint64_t i = 0; i < a.iChildCount; i++) {
        uint64_t child;
        if (xuiDocumentSnapshotGetChild(old, node, i, &child) != XUI_OK || !all_ids(old, now, child, moving, new_parent)) return 0;
    }
    return 1;
}
static void source_variant(const char* source, char* output, unsigned eol, unsigned bom, unsigned eof)
{
    static const char* endings[] = {"\n", "\r\n", "\r"};
    size_t length = strlen(source), at = 0;
    if (!eof && length && source[length - 1] == '\n') length--;
    if (bom) { memcpy(output, "\xef\xbb\xbf", 3); at = 3; }
    for (size_t i = 0; i < length; i++) {
        if (source[i] == '\n') { size_t n = strlen(endings[eol]); memcpy(output + at, endings[eol], n); at += n; }
        else output[at++] = source[i];
    }
    output[at] = 0;
}
#ifndef BLOCK_KINDS_MATRIX_NO_MAIN
int main(void)
{
    unsigned fixture_failures = 0, rejections = 0, raw_failures = 0, semantic_failures = 0, history_failures = 0;
    for (unsigned variant = 0; variant < 12 * sizeof(cases) / sizeof(*cases); variant++) {
        unsigned i = variant / 12, v = variant % 12;
        char before[2048], after[2048]; block_case transformed = cases[i];
        source_variant(cases[i].before, before, v % 3, (v / 3) % 2, v / 6);
        source_variant(cases[i].after, after, v % 3, (v / 3) % 2, strstr(cases[i].name, "-reverse") ? 1 : v / 6);
        transformed.before = before; transformed.after = after;
        const block_case* c = &transformed; xui_document d = NULL, oracle = NULL;
        xui_document_snapshot old = NULL, expected = NULL, now = NULL, restored = NULL;
        xui_document_transaction t = NULL; xui_doc_node_info_t target = {0}; target.iSize = sizeof(target);
        uint64_t moving = 0, parent = 0, wanted_moving = 0, wanted_parent = 0, child = 0, na = 0, nb = 0;
        char *a = NULL, *b = NULL; int raw = 0, semantic = 0, history = 0;
        int result = open_document(c->before, &d), fixture = 1;
        if (result == XUI_OK) result = xuiDocumentAcquireSnapshot(d, &old);
        if (result == XUI_OK) result = moving_node(old, c->before, c->moving_kind, &moving);
        if (result == XUI_OK) result = target_node(old, c->before, c->target_kind, &parent);
        if (result == XUI_OK) result = xuiDocumentSnapshotGetNode(old, parent, &target);
        if (result == XUI_OK && c->target_empty && target.iChildCount) result = -1;
        if (result == XUI_OK) {
            xui_doc_node_info_t child_info = {0}, old_parent = {0};
            child_info.iSize = old_parent.iSize = sizeof(child_info);
            result = xuiDocumentSnapshotGetNode(old, moving, &child_info);
            if (result == XUI_OK) result = xuiDocumentSnapshotGetNode(old, child_info.iParentId, &old_parent);
            if (result == XUI_OK && (old_parent.iChildCount != 1 || old_parent.iKind == XUI_DOC_ROOT)) result = -1;
        }
        if (result == XUI_OK) result = open_document(c->after, &oracle);
        if (result == XUI_OK) result = xuiDocumentAcquireSnapshot(oracle, &expected);
        if (result == XUI_OK) result = moving_node(expected, c->after, c->moving_kind, &wanted_moving);
        if (result == XUI_OK) result = target_node(expected, c->after, c->target_kind, &wanted_parent);
        if (result == XUI_OK) result = xuiDocumentSnapshotGetChild(expected, wanted_parent, c->index, &child);
        if (result == XUI_OK && child != wanted_moving) result = -1;
        if (result != XUI_OK) { fixture = 0; fixture_failures++; }
        if (result == XUI_OK) {
            uint64_t ancestor = parent; xui_doc_node_info_t node = {0}; node.iSize = sizeof(node);
            while (ancestor) {
                if (ancestor == moving || xuiDocumentSnapshotGetNode(old, ancestor, &node) != XUI_OK) { result = -1; fixture = 0; fixture_failures++; break; }
                ancestor = node.iParentId;
            }
        }
        if (result == XUI_OK) result = xuiDocumentBeginTransaction(d, NULL, &t);
        if (result == XUI_OK) result = xuiDocumentTxnMoveNode(t, moving, parent, c->index);
        if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
        xuiDocumentTxnRelease(t);
        if (result == XUI_OK) result = xuiDocumentAcquireSnapshot(d, &now);
        if (result == XUI_OK) {
            raw = source_equal(now, c->after);
            if (xuiDocumentExportHtml(now, &a, &na) == XUI_OK && xuiDocumentExportHtml(expected, &b, &nb) == XUI_OK)
                semantic = na == nb && !memcmp(a, b, (size_t)na);
            history = source_equal(old, c->before) && all_ids(old, now, 1, moving, parent) &&
                xuiDocumentSnapshotGetChild(now, parent, c->index, &child) == XUI_OK && child == moving;
            if (xuiDocumentUndo(d, NULL) != XUI_OK || xuiDocumentAcquireSnapshot(d, &restored) != XUI_OK ||
                !source_equal(restored, c->before) || !all_ids(old, restored, 1, 0, 0)) history = 0;
            xuiDocumentSnapshotRelease(restored); restored = NULL;
            if (xuiDocumentRedo(d, NULL) != XUI_OK || xuiDocumentAcquireSnapshot(d, &restored) != XUI_OK ||
                !source_equal(restored, c->after) || !all_ids(old, restored, 1, moving, parent)) history = 0;
        }
        rejections += fixture && result != XUI_OK;
        raw_failures += result == XUI_OK && !raw;
        semantic_failures += result == XUI_OK && !semantic;
        history_failures += result == XUI_OK && !history;
        printf("Block kind case=%u name=%s fixture=%d target-children=%llu result=%d raw=%d semantic=%d history=%d\n",
            i, c->name, fixture, (unsigned long long)target.iChildCount, result, raw, semantic, history);
        if (result == XUI_OK && !raw) {
            char source[4096]; uint64_t bytes;
            if (xuiDocumentSnapshotCopySource(now, source, sizeof(source) - 1, &bytes) == XUI_OK) {
                source[bytes] = 0; printf("Actual=[%s]\nWanted=[%s]\n", source, c->after);
            }
        }
        xuiDocumentFreeBuffer(a); xuiDocumentFreeBuffer(b); xuiDocumentSnapshotRelease(restored);
        xuiDocumentSnapshotRelease(now); xuiDocumentSnapshotRelease(expected); xuiDocumentSnapshotRelease(old);
        xuiDocumentRelease(oracle); xuiDocumentRelease(d);
    }
    printf("Block kinds: 1260 independent sole-child oracle variants; %u fixture failures, %u rejections, %u raw differences, %u semantic differences, %u history differences\n",
        fixture_failures, rejections, raw_failures, semantic_failures, history_failures);
    return fixture_failures || rejections || raw_failures || semantic_failures || history_failures ? 1 : 0;
}

#endif
