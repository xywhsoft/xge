#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_W 1080
#define DEMO_H 720
#define OBJECT_TOOL_COUNT 4
#define DEMO_INSERT_LINK 1001
#define DEMO_INSERT_IMAGE 1002
#define DEMO_INSERT_TABLE 1003
#define DEMO_INSERT_RULE 1004

typedef struct rich_demo_t rich_demo_t;

typedef struct rich_tool_t {
	rich_demo_t* demo;
	xui_widget button;
	int command;
} rich_tool_t;

struct rich_demo_t {
	xui_proxy_t proxy;
	xui_context context;
	xui_surface target;
	xui_surface image;
	xui_font font;
	xui_font boldFont;
	xui_font italicFont;
	xui_font boldItalicFont;
	xui_widget root;
	xui_widget edit;
	xui_widget toolbar;
	xui_widget action;
	xui_widget status;
	rich_tool_t tools[OBJECT_TOOL_COUNT];
};

static const char* find_font(const char* const* paths, int count)
{
	FILE* file;
	int i;
	for (i = 0; i < count; i++) {
		file = fopen(paths[i], "rb");
		if (file != NULL) { fclose(file); return paths[i]; }
	}
	return NULL;
}

static int root_render(xui_widget widget, xui_draw_context draw, uint32_t state, void* user)
{
	rich_demo_t* demo = (rich_demo_t*)user;
	(void)state;
	return demo->proxy.drawRectFill(&demo->proxy, draw, xuiWidgetGetContentRect(widget), XUI_COLOR_RGBA(238,244,250,255));
}

static void set_status(rich_demo_t* demo, const char* text)
{
	if (demo->status != NULL) (void)xuiLabelSetText(demo->status, text);
}

static void action_click(xui_widget widget, void* user)
{
	rich_demo_t* demo = (rich_demo_t*)user;
	xui_rich_text_style_t style;
	(void)widget;
	memset(&style, 0, sizeof(style));
	style.iSize = sizeof(style);
	style.iTextColor = XUI_COLOR_RGBA(19,112,92,255);
	style.iBackgroundColor = XUI_COLOR_RGBA(220,248,237,255);
	style.iFlags = XUI_RICH_STYLE_UNDERLINE;
	(void)xuiRichEditApplyStyle(demo->edit, &style);
	(void)xuiSetFocusWidget(demo->context, demo->edit);
	set_status(demo, "Embedded widget event: accent style applied to the selection.");
}

static void link_click(xui_widget widget, xui_document_node_id_t nodeId, const char* url, void* user)
{
	rich_demo_t* demo = (rich_demo_t*)user;
	char text[320];
	(void)widget;
	snprintf(text, sizeof(text), "Link event: node %llu, %s", (unsigned long long)nodeId, url != NULL ? url : "");
	set_status(demo, text);
}

static void refresh_tools(rich_demo_t* demo)
{
	if (demo->edit != NULL && demo->toolbar != NULL)
		(void)xuiRichEditSyncToolbar(demo->edit, demo->toolbar);
}

static void rich_event(xui_widget widget, const xui_rich_edit_event_t* event, void* user)
{
	rich_demo_t* demo = (rich_demo_t*)user;
	char text[160];
	(void)widget;
	refresh_tools(demo);
	if (event != NULL && event->iType == XUI_RICH_EDIT_EVENT_ZOOM_CHANGED) {
		snprintf(text, sizeof(text), "Zoom %.0f%%", event->fZoom * 100.0f);
		set_status(demo, text);
	}
}

static void toolbar_select(xui_widget toolbar, int index, int value, void* user)
{
	rich_demo_t* demo = (rich_demo_t*)user;
	(void)value;
	(void)xuiRichEditExecuteToolbarItem(demo->edit, toolbar, index);
	(void)xuiSetFocusWidget(demo->context, demo->edit);
}

static void tool_click(xui_widget widget, void* user)
{
	rich_tool_t* tool = (rich_tool_t*)user;
	rich_demo_t* demo = tool->demo;
	(void)widget;
	if (tool->command == DEMO_INSERT_LINK) {
		(void)xuiRichEditApplyLink(demo->edit, "https://github.com/xge-project/");
	} else if (tool->command == DEMO_INSERT_IMAGE) {
		xui_rich_image_desc_t image;
		memset(&image, 0, sizeof(image)); image.iSize = sizeof(image);
		image.pSurface = demo->image; image.sSource = "demo://color-study"; image.sAltText = "Embedded color study";
		image.fWidth = 360; image.fHeight = 112; image.iAlign = XUI_RICH_ALIGN_CENTER;
		(void)xuiRichEditInsertImage(demo->edit, &image);
	} else if (tool->command == DEMO_INSERT_TABLE) {
		xui_rich_table_desc_t table;
		memset(&table, 0, sizeof(table)); table.iSize = sizeof(table); table.iRows = 3; table.iColumns = 3;
		table.fWidth = 560; table.fCellPadding = 6; table.fBorderWidth = 1;
		table.iBorderColor = XUI_COLOR_RGBA(170,184,201,255); table.iHeaderColor = XUI_COLOR_RGBA(225,234,244,255);
		table.iCellColor = XUI_COLOR_RGBA(250,252,254,255);
		(void)xuiRichEditInsertTable(demo->edit, &table);
	} else if (tool->command == DEMO_INSERT_RULE) {
		(void)xuiRichEditInsertHorizontalRule(demo->edit);
	} else {
		(void)xuiRichEditExecuteCommand(demo->edit, tool->command, NULL);
	}
	(void)xuiSetFocusWidget(demo->context, demo->edit);
	refresh_tools(demo);
}

static int create_toolbar(rich_demo_t* demo)
{
	static const struct { const char* text; int command; } tools[OBJECT_TOOL_COUNT] = {
		{"Link",DEMO_INSERT_LINK},{"Image",DEMO_INSERT_IMAGE},
		{"Table",DEMO_INSERT_TABLE},{"Rule",DEMO_INSERT_RULE}
	};
	xui_toolbar_desc_t toolbarDesc;
	float x = 18.0f;
	int i;
	int ret;
	memset(&toolbarDesc, 0, sizeof(toolbarDesc)); toolbarDesc.iSize = sizeof(toolbarDesc); toolbarDesc.pFont = demo->font;
	ret = xuiToolbarCreate(demo->context, &demo->toolbar, &toolbarDesc); if (ret != XUI_OK) return ret;
	(void)xuiWidgetSetRect(demo->toolbar, (xui_rect_t){18,10,DEMO_W-36,34});
	(void)xuiToolbarSetItemSize(demo->toolbar, 66.0f, 30.0f, 8.0f);
	(void)xuiToolbarSetOverflow(demo->toolbar, 1, 30.0f, NULL, NULL);
	ret = xuiRichEditSetupToolbar(demo->edit, demo->toolbar, XUI_RICH_TOOLBAR_DEFAULT);
	if (ret != XUI_OK) return ret;
	(void)xuiToolbarSetSelect(demo->toolbar, toolbar_select, demo);
	ret = xuiWidgetAddChild(demo->root, demo->toolbar); if (ret != XUI_OK) return ret;
	for (i = 0; i < OBJECT_TOOL_COUNT; i++) {
		xui_button_desc_t desc;
		size_t length = strlen(tools[i].text);
		float width = length <= 2 ? 36.0f : length <= 4 ? 52.0f : length <= 6 ? 66.0f : 76.0f;
		memset(&desc, 0, sizeof(desc)); desc.iSize = sizeof(desc); desc.sText = tools[i].text; desc.pFont = demo->font;
		demo->tools[i].demo = demo; demo->tools[i].command = tools[i].command;
		if (xuiButtonCreate(demo->context, &demo->tools[i].button, &desc) != XUI_OK) return XUI_ERROR_OUT_OF_MEMORY;
		(void)xuiButtonSetSelectable(demo->tools[i].button, 1);
		(void)xuiButtonSetClick(demo->tools[i].button, tool_click, &demo->tools[i]);
		(void)xuiWidgetSetRect(demo->tools[i].button, (xui_rect_t){x,48.0f,width,30});
		(void)xuiWidgetAddChild(demo->root, demo->tools[i].button);
		x += width + 4.0f;
	}
	return XUI_OK;
}

static int create_demo_image(rich_demo_t* demo)
{
	unsigned char* pixels;
	int x;
	int y;
	int ret;
	const int width = 360;
	const int height = 112;
	pixels = (unsigned char*)malloc((size_t)width * (size_t)height * 4u);
	if (pixels == NULL) return XUI_ERROR_OUT_OF_MEMORY;
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			unsigned char* p = pixels + ((size_t)y * (size_t)width + (size_t)x) * 4u;
			int band = x * 3 / width;
			p[0] = (unsigned char)(band == 0 ? 45 : band == 1 ? 236 : 54);
			p[1] = (unsigned char)(band == 0 ? 126 : band == 1 ? 111 : 177);
			p[2] = (unsigned char)(band == 0 ? 214 : band == 1 ? 76 : 112);
			p[3] = 255;
			if (((x + y) % 24) == 0) { p[0] = 250; p[1] = 250; p[2] = 250; }
		}
	}
	ret = demo->proxy.surfaceCreateRGBA(&demo->proxy, &demo->image, width, height, pixels, width * 4, XUI_SURFACE_ALPHA_STRAIGHT);
	free(pixels);
	return ret;
}

static int create_ui(rich_demo_t* demo)
{
	xui_rich_document document;
	xui_rich_node paragraph;
	xui_rich_text_style_t title;
	xui_rich_text_style_t body;
	xui_rich_text_style_t accent;
	xui_rich_text_style_t linkStyle;
	xui_rich_image_desc_t image;
	xui_rich_font_set_t fonts;
	xui_rich_edit_desc_t editDesc;
	xui_button_desc_t buttonDesc;
	xui_label_desc_t labelDesc;
	int ret;

	ret = xuiWidgetCreate(demo->context, &demo->root); if (ret != XUI_OK) return ret;
	(void)xuiWidgetSetRect(demo->root, (xui_rect_t){0,0,DEMO_W,DEMO_H});
	(void)xuiWidgetSetCacheRenderCallback(demo->root, root_render, demo);
	ret = xuiSetRootWidget(demo->context, demo->root); if (ret != XUI_OK) return ret;
	memset(&labelDesc, 0, sizeof(labelDesc)); labelDesc.iSize = sizeof(labelDesc); labelDesc.pFont = demo->font;
	labelDesc.sText = "Edit text, use the command toolbar, activate links and widgets, or insert structured objects.";
	labelDesc.iTextColor = XUI_COLOR_RGBA(70,88,110,255);
	ret = xuiLabelCreate(demo->context, &demo->status, &labelDesc); if (ret != XUI_OK) return ret;
	(void)xuiWidgetSetRect(demo->status, (xui_rect_t){20,82,DEMO_W-40,24});
	(void)xuiWidgetAddChild(demo->root, demo->status);
	memset(&buttonDesc, 0, sizeof(buttonDesc)); buttonDesc.iSize = sizeof(buttonDesc);
	buttonDesc.sText = "Apply accent"; buttonDesc.pFont = demo->font;
	ret = xuiButtonCreate(demo->context, &demo->action, &buttonDesc); if (ret != XUI_OK) return ret;
	(void)xuiButtonSetClick(demo->action, action_click, demo);
	ret = xuiRichDocumentCreate(&document); if (ret != XUI_OK) return ret;
	paragraph = xuiRichNodeGetFirstChild(xuiRichDocumentGetRoot(document));
	memset(&title, 0, sizeof(title)); title.iSize = sizeof(title); title.iTextColor = XUI_COLOR_RGBA(25,56,96,255); title.iFlags = XUI_RICH_STYLE_BOLD;
	memset(&body, 0, sizeof(body)); body.iSize = sizeof(body); body.iTextColor = XUI_COLOR_RGBA(55,69,88,255);
	memset(&accent, 0, sizeof(accent)); accent.iSize = sizeof(accent); accent.iTextColor = XUI_COLOR_RGBA(178,72,28,255); accent.iFlags = XUI_RICH_STYLE_ITALIC;
	memset(&linkStyle, 0, sizeof(linkStyle)); linkStyle.iSize = sizeof(linkStyle); linkStyle.iTextColor = XUI_COLOR_RGBA(31,104,196,255); linkStyle.iFlags = XUI_RICH_STYLE_UNDERLINE;
	(void)xuiRichDocumentAppendText(document, paragraph, "XUI RichEdit - structured, interactive editing", &title);
	paragraph = xuiRichDocumentAppendParagraph(document);
	(void)xuiRichDocumentAppendText(document, paragraph, "Text, styles and arbitrary XUI widgets share one flow. This is a live control: ", &body);
	(void)xuiRichDocumentAppendWidget(document, paragraph, demo->action, 132.0f, 30.0f, 22.0f);
	(void)xuiRichDocumentAppendText(document, paragraph, "  This is a document link: ", &body);
	(void)xuiRichDocumentAppendLink(document, paragraph, "XGE project", "https://github.com/xge-project/", &linkStyle);
	(void)xuiRichDocumentAppendText(document, paragraph, ". Both dispatch real events without leaving the editor.", &body);
	memset(&image, 0, sizeof(image)); image.iSize = sizeof(image); image.pSurface = demo->image;
	image.sSource = "demo://color-study"; image.sAltText = "Embedded color study";
	image.fWidth = 360; image.fHeight = 112; image.iAlign = XUI_RICH_ALIGN_CENTER;
	(void)xuiRichDocumentAppendImage(document, &image);
	paragraph = xuiRichDocumentAppendParagraph(document);
	(void)xuiRichDocumentAppendText(document, paragraph, "The image above is a real surface-backed document node. ", &body);
	(void)xuiRichDocumentAppendText(document, paragraph, "Select this italic text", &accent);
	(void)xuiRichDocumentAppendText(document, paragraph, " and toggle bold, italic or underline. Chinese, \xe4\xb8\xad\xe6\x96\x87 and emoji \xf0\x9f\x98\x84 are measured as UTF-8 grapheme fragments.", &body);
	memset(&editDesc, 0, sizeof(editDesc)); editDesc.iSize = sizeof(editDesc); editDesc.pDocument = document;
	editDesc.bOwnDocument = 1; editDesc.pFont = demo->font; editDesc.bWordWrap = 1;
	ret = xuiRichEditCreate(demo->context, &demo->edit, &editDesc);
	if (ret != XUI_OK) { xuiRichDocumentDestroy(document); return ret; }
	memset(&fonts, 0, sizeof(fonts)); fonts.iSize = sizeof(fonts); fonts.pNormal = demo->font;
	fonts.pBold = demo->boldFont; fonts.pItalic = demo->italicFont; fonts.pBoldItalic = demo->boldItalicFont;
	(void)xuiRichEditSetFontSet(demo->edit, &fonts);
	(void)xuiRichEditSetLinkClick(demo->edit, link_click, demo);
	(void)xuiRichEditSetEvent(demo->edit, rich_event, demo);
	(void)xuiWidgetSetAccessibleName(demo->edit, "Rich text editing document");
	(void)xuiWidgetSetRect(demo->edit, (xui_rect_t){18,108,DEMO_W-36,DEMO_H-126});
	ret = xuiWidgetAddChild(demo->root, demo->edit);
	if (ret == XUI_OK) ret = create_toolbar(demo);
	if (ret == XUI_OK) refresh_tools(demo);
	return ret;
}

static int frame(void* user)
{
	rich_demo_t* demo = (rich_demo_t*)user;
	xui_rect_i_t full = {0,0,DEMO_W,DEMO_H};
	xui_rect_t src = {0,0,DEMO_W,DEMO_H};
	xui_rect_t dst = {0,0,DEMO_W,DEMO_H};
	int ret;
	if (xgeKeyPressed(XGE_KEY_ESCAPE)) xgeQuit();
	ret = xgeBegin(); if (ret != XGE_OK) return ret;
	ret = xuiProxyXgePumpInputRect(demo->context, (xui_rect_t){0,0,DEMO_W,DEMO_H}); if (ret != XUI_OK) return ret;
	ret = xuiDispatchPendingEvents(demo->context); if (ret != XUI_OK) return ret;
	refresh_tools(demo);
	ret = xuiLayout(demo->context); if (ret != XUI_OK) return ret;
	ret = xuiUpdate(demo->context, xgeGetDelta()); if (ret != XUI_OK) return ret;
	ret = demo->proxy.surfaceClear(&demo->proxy, demo->target, XUI_COLOR_RGBA(238,244,250,255)); if (ret != XUI_OK) return ret;
	ret = xuiRender(demo->context, demo->target, &full, 1); if (ret != XUI_OK) return ret;
	xgeClear(XUI_COLOR_RGBA(23,29,38,255));
	ret = demo->proxy.surfaceDraw(&demo->proxy, demo->target, src, dst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	return ret == XUI_OK ? xgeEnd() : ret;
}

int main(void)
{
	static const char* normalPaths[] = {"C:\\Windows\\Fonts\\segoeui.ttf", "C:\\Windows\\Fonts\\msyh.ttc"};
	static const char* boldPaths[] = {"C:\\Windows\\Fonts\\segoeuib.ttf", "C:\\Windows\\Fonts\\msyhbd.ttc"};
	static const char* italicPaths[] = {"C:\\Windows\\Fonts\\segoeuii.ttf", "C:\\Windows\\Fonts\\segoeui.ttf"};
	static const char* boldItalicPaths[] = {"C:\\Windows\\Fonts\\segoeuiz.ttf", "C:\\Windows\\Fonts\\segoeuib.ttf"};
	rich_demo_t demo;
	xge_desc_t desc;
	xui_surface_desc_t surfaceDesc;
	const char* path;
	int ret;
	memset(&demo, 0, sizeof(demo));
	memset(&desc, 0, sizeof(desc)); desc.iWidth = DEMO_W; desc.iHeight = DEMO_H; desc.sTitle = "XUI RichEdit";
	desc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC; desc.iRunMode = XGE_RUN_GAME_LOOP; desc.iTargetFPS = 60;
	ret = xgeInit(&desc); if (ret != XGE_OK) return 1;
	demo.proxy = xuiProxyXge();
	ret = xuiCreate(&demo.context); if (ret != XUI_OK) return 1;
	ret = xuiSetProxy(demo.context, &demo.proxy); if (ret != XUI_OK) return 1;
	path = find_font(normalPaths, 2); if (path == NULL || demo.proxy.fontLoadFile(&demo.proxy, &demo.font, path, 18.0f, XUI_FONT_FORMAT_TTF) != XUI_OK) return 1;
	path = find_font(boldPaths, 2); if (path == NULL || demo.proxy.fontLoadFile(&demo.proxy, &demo.boldFont, path, 18.0f, XUI_FONT_FORMAT_TTF) != XUI_OK) return 1;
	path = find_font(italicPaths, 2); if (path == NULL || demo.proxy.fontLoadFile(&demo.proxy, &demo.italicFont, path, 18.0f, XUI_FONT_FORMAT_TTF) != XUI_OK) return 1;
	path = find_font(boldItalicPaths, 2); if (path == NULL || demo.proxy.fontLoadFile(&demo.proxy, &demo.boldItalicFont, path, 18.0f, XUI_FONT_FORMAT_TTF) != XUI_OK) return 1;
	(void)xuiSetDefaultFont(demo.context, demo.font);
	(void)xuiInputViewport(demo.context, DEMO_W, DEMO_H);
	memset(&surfaceDesc, 0, sizeof(surfaceDesc)); surfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	surfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8; surfaceDesc.iWidth = DEMO_W; surfaceDesc.iHeight = DEMO_H;
	surfaceDesc.iFlags = XUI_SURFACE_USAGE_TARGET | XUI_SURFACE_ALPHA_PREMULTIPLIED;
	ret = demo.proxy.surfaceCreate(&demo.proxy, &demo.target, &surfaceDesc); if (ret != XUI_OK) return 1;
	ret = create_demo_image(&demo); if (ret != XUI_OK) return 1;
	ret = create_ui(&demo); if (ret != XUI_OK) return 1;
	ret = xgeRun(frame, &demo);
	xuiDestroy(demo.context);
	demo.proxy.surfaceDestroy(&demo.proxy, demo.image);
	demo.proxy.surfaceDestroy(&demo.proxy, demo.target);
	demo.proxy.fontDestroy(&demo.proxy, demo.boldItalicFont);
	demo.proxy.fontDestroy(&demo.proxy, demo.italicFont);
	demo.proxy.fontDestroy(&demo.proxy, demo.boldFont);
	demo.proxy.fontDestroy(&demo.proxy, demo.font);
	xgeUnit();
	return ret == XGE_OK ? 0 : 1;
}
