#include "../xui_document_ui.h"
#include "../xge.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "DIB paste failed at %d: %s\n", __LINE__, #expr); \
    failed = 1; goto done; \
} } while (0)

static xui_doc_node_id find_image(xui_document_snapshot snapshot, xui_doc_node_id parent)
{
    xui_doc_node_info_t info = {0};
    xui_doc_node_id child, found;
    uint64_t i;
    info.iSize = sizeof(info);
    if (xuiDocumentSnapshotGetNode(snapshot, parent, &info) != XUI_OK) return 0;
    if (info.iKind == XUI_DOC_IMAGE) return parent;
    for (i = 0; i < info.iChildCount; i++) {
        if (xuiDocumentSnapshotGetChild(snapshot, parent, i, &child) != XUI_OK) return 0;
        found = find_image(snapshot, child);
        if (found) return found;
    }
    return 0;
}

static int set_dib_clipboard(HWND owner)
{
    static const unsigned char pixel[4] = {20u, 90u, 210u, 170u};
    HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPV5HEADER) + sizeof(pixel));
    BITMAPV5HEADER* header;
    if (!memory) return 0;
    header = (BITMAPV5HEADER*)GlobalLock(memory);
    if (!header) { GlobalFree(memory); return 0; }
    memset(header, 0, sizeof(*header));
    header->bV5Size = sizeof(*header);
    header->bV5Width = header->bV5Height = 1;
    header->bV5Planes = 1; header->bV5BitCount = 32;
    header->bV5Compression = BI_BITFIELDS;
    header->bV5SizeImage = sizeof(pixel);
    header->bV5RedMask = 0x00ff0000u;
    header->bV5GreenMask = 0x0000ff00u;
    header->bV5BlueMask = 0x000000ffu;
    header->bV5AlphaMask = 0xff000000u;
    memcpy((unsigned char*)header + sizeof(*header), pixel, sizeof(pixel));
    GlobalUnlock(memory);
    if (!OpenClipboard(owner)) { GlobalFree(memory); return 0; }
    if (!EmptyClipboard() || !SetClipboardData(CF_DIBV5, memory)) {
        CloseClipboard(); GlobalFree(memory); return 0;
    }
    CloseClipboard(); return 1;
}

static int copied_dib_matches(void)
{
    static const unsigned char pixel[4] = {20u, 90u, 210u, 170u};
    HGLOBAL memory;
    const BITMAPV5HEADER* header;
    int match;
    if (!IsClipboardFormatAvailable(CF_DIBV5) || !OpenClipboard(NULL)) return 0;
    memory = (HGLOBAL)GetClipboardData(CF_DIBV5);
    header = memory ? (const BITMAPV5HEADER*)GlobalLock(memory) : NULL;
    match = header && GlobalSize(memory) >= sizeof(*header) + sizeof(pixel) &&
        header->bV5Size == sizeof(*header) && header->bV5Width == 1 &&
        header->bV5Height == -1 && header->bV5BitCount == 32 &&
        header->bV5Compression == BI_BITFIELDS &&
        memcmp((const unsigned char*)header + sizeof(*header), pixel,
            sizeof(pixel)) == 0;
    if (header) GlobalUnlock(memory);
    CloseClipboard();
    return match;
}

int main(void)
{
    xge_desc_t xge = {0};
    xui_proxy_t proxy = {0};
    xui_context context = NULL;
    xui_font font = NULL;
    xui_document document = NULL;
    xui_widget editor = NULL;
    xui_doc_editor_desc_t desc = {0};
    xui_document_snapshot snapshot = NULL;
    xui_doc_range_t selection;
    char* original_text = NULL;
    char name[80];
    uint64_t original_revision, found;
    xui_doc_node_id image_id;
    xui_doc_node_id child_id;
    xui_doc_node_info_t info = {0};
    xui_doc_node_info_t parent = {0};
    uint64_t child_index;
    HWND owner = NULL;
    const char *saved;
    int failed = 0, xge_ready = 0;
    xge.iRunMode = XGE_RUN_MANUAL;
    CHECK(xgeInit(&xge) == XGE_OK);
    xge_ready = 1;
    saved = xgeClipboardGetText();
    original_text = (char*)malloc(strlen(saved) + 1u);
    CHECK(original_text != NULL);
    strcpy(original_text, saved);
    owner = CreateWindowExA(0, "STATIC", "XUI DIB paste test", WS_POPUP,
        0, 0, 0, 0, NULL, NULL, GetModuleHandleA(NULL), NULL);
    CHECK(owner != NULL);
    proxy = xuiProxyXge();
    CHECK(xuiCreate(&context) == XUI_OK &&
        xuiSetProxy(context, &proxy) == XUI_OK &&
        proxy.fontLoadFile(&proxy, &font,
            "C:\\Windows\\Fonts\\segoeui.ttf", 14, XUI_FONT_FORMAT_TTF) == XUI_OK &&
        xuiSetDefaultFont(context, font) == XUI_OK);
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView);
    desc.tView.pDocument = document; desc.iMode = XUI_DOC_VISUAL;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK &&
        xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 180}) == XUI_OK);
    CHECK(xuiDocumentEditorInsertText(editor, "XY", 2) == XUI_OK &&
        xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "XY", 2,
            NULL, &selection, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot); snapshot = NULL;
    selection.tAnchor = selection.tCaret;
    selection.tAnchor.iOffset = 1; selection.tCaret = selection.tAnchor;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    original_revision = xuiDocumentGetRevision(document);
    CHECK(set_dib_clipboard(owner) &&
        xgeClipboardGetData(XGE_CLIPBOARD_FORMAT_IMAGE_PNG, NULL, 0u) > 8 &&
        xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_PASTE) == XUI_OK &&
        xuiDocumentGetRevision(document) == original_revision + 1 &&
        xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    image_id = find_image(snapshot, XUI_DOCUMENT_ROOT);
    CHECK(image_id != 0);
    info.iSize = sizeof(info);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, image_id, &info) == XUI_OK &&
        info.sResource != NULL && strlen(info.sResource) < sizeof(name));
    strcpy(name, info.sResource);
    CHECK(strncmp(name, "xui-clipboard-image-", 20) == 0);
    CHECK(xuiResourceFind(context, name) != NULL);
    parent.iSize = sizeof(parent);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, info.iParentId, &parent) == XUI_OK);
    for (child_index = 0; child_index < parent.iChildCount; child_index++) {
        CHECK(xuiDocumentSnapshotGetChild(snapshot, parent.iId,
            child_index, &child_id) == XUI_OK);
        if (child_id == image_id) break;
    }
    CHECK(child_index < parent.iChildCount);
    memset(&selection, 0, sizeof(selection));
    selection.tAnchor.iSize = selection.tCaret.iSize = sizeof(selection.tAnchor);
    selection.tAnchor.iKind = selection.tCaret.iKind = XUI_DOC_POSITION_GAP;
    selection.tAnchor.iDocumentId = selection.tCaret.iDocumentId =
        xuiDocumentGetIdentity(document);
    selection.tAnchor.iRevision = selection.tCaret.iRevision =
        xuiDocumentGetRevision(document);
    selection.tAnchor.iNodeId = selection.tCaret.iNodeId = parent.iId;
    selection.tAnchor.iOffset = child_index;
    selection.tCaret.iOffset = child_index + 1;
    selection.tAnchor.iAffinity = selection.tCaret.iAffinity = XUI_DOC_AFTER;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_COPY) == XUI_OK &&
        copied_dib_matches());
    xuiDocumentSnapshotRelease(snapshot); snapshot = NULL;
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK &&
        xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        find_image(snapshot, XUI_DOCUMENT_ROOT) == 0);
    xuiDocumentSnapshotRelease(snapshot); snapshot = NULL;
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK &&
        xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        find_image(snapshot, XUI_DOCUMENT_ROOT) != 0);
done:
    if (original_text && xge_ready) xgeClipboardSetText(original_text);
    if (owner) DestroyWindow(owner);
    free(original_text);
    xuiDocumentSnapshotRelease(snapshot);
    if (context) xuiDestroy(context);
    xuiDocumentRelease(document);
    if (font && proxy.fontDestroy) proxy.fontDestroy(&proxy, font);
    if (xge_ready) xgeUnit();
    if (failed) return 1;
    puts("Document VISUAL Win32 CF_DIBV5 paste/copy, resource, Undo and Redo passed");
    return 0;
}
