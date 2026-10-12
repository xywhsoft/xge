#ifndef PE_THEME_H
#define PE_THEME_H
#include <stdint.h>

enum
{
	PE_THEME_DARK,
	PE_THEME_LIGHT,
	PE_THEME_COUNT
};
enum
{
	PE_BG,
	PE_PANEL,
	PE_FIELD,
	PE_LINE,
	PE_TEXT,
	PE_MUTED,
	PE_DISABLED,
	PE_ACCENT,
	PE_SELECTED,
	PE_HOVER,
	PE_ACTIVE,
	PE_ON_ACCENT,
	PE_BUTTON,
	PE_SCROLL,
	PE_SCROLL_HOVER,
	PE_ERROR,
	PE_WARNING_BG,
	PE_WARNING_TEXT,
	PE_CANVAS,
	PE_GRID,
	PE_CURVE,
	PE_BURST,
	PE_SPAN,
	PE_CHECKER_A,
	PE_CHECKER_B,
	PE_SHADOW,
	PE_TRANSPARENT,
	PE_COLOR_COUNT
};
struct pe_app;
const uint32_t *pe_theme_colors(int theme);
int pe_theme_init(struct pe_app *app);
int pe_theme_apply(struct pe_app *app, int theme);
void pe_theme_preferences_load(struct pe_app *app);
void pe_theme_preferences_save(struct pe_app *app);
#endif
