#ifndef XUI_DOCUMENT_MD4C_H
#define XUI_DOCUMENT_MD4C_H
#include "xui_document_internal.h"
#include "../lib/md4c/md4c.h"
int doc_md4c_parse(doc_allocator*, const char*, MD_SIZE, const MD_PARSER*, void*);
#endif
