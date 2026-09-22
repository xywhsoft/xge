@rem Unified Document core. No window, renderer, or widget dependency.
@rem Keep every consuming build on this single source manifest.
set XUI_DOCUMENT_SRC=src\xui_document_store.c src\xui_document.c src\xui_document_schema.c src\xui_document_position.c src\xui_document_range.c src\xui_document_commands.c src\xui_document_markdown.c src\xui_document_reconcile.c src\xui_document_md4c.c src\xui_document_io.c src\xui_document_html.c src\xui_document_equivalent.c lib\md4c\entity.c
set XUI_DOCUMENT_SRC=%XUI_DOCUMENT_SRC% src\xui_document_table.c
set XUI_DOCUMENT_SRC=%XUI_DOCUMENT_SRC% src\xui_document_markdown_edit.c
set XUI_DOCUMENT_SRC=%XUI_DOCUMENT_SRC% src\xui_document_search.c
set XUI_DOCUMENT_SRC=%XUI_DOCUMENT_SRC% src\xui_document_file.c
