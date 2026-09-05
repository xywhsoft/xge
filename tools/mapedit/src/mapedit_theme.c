#include "mapedit_theme.h"
#include <stdio.h>

int mapeditThemeInit(xui_context context, xui_proxy_t* proxy, xui_font* font, const char* appDir)
{
	char path[1024];
	xui_theme_t theme;
	int ret;
	snprintf(path, sizeof(path), "%s\\res\\fonts\\Consolas-with-Yahei_Nerd_Font.ttf", appDir);
	ret = proxy->fontLoadFile(proxy, font, path, MAPEDIT_FONT_SIZE, XUI_FONT_FORMAT_TTF);
	if (ret != XUI_OK) ret = proxy->fontLoadFile(proxy, font, "C:\\Windows\\Fonts\\msyh.ttc", MAPEDIT_FONT_SIZE, XUI_FONT_FORMAT_TTF);
	if (ret != XUI_OK) return ret;
	xuiThemeDefault(&theme);
	theme.pFont = *font;
	theme.iTextColor = XUI_COLOR_RGBA(25, 45, 64, 255);
	theme.iBackgroundColor = XUI_COLOR_RGBA(232, 241, 250, 255);
	theme.iPanelColor = XUI_COLOR_RGBA(248, 252, 255, 255);
	theme.iAccentColor = XUI_COLOR_RGBA(38, 117, 178, 255);
	ret = xuiSetTheme(context, &theme);
	if (ret == XUI_OK) ret = xuiSetDefaultFont(context, *font);
	if (ret == XUI_OK) ret = xuiSetLanguage(context, XUI_LANGUAGE_ZH);
	return ret;
}
