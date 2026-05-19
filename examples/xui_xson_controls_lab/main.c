#include "../../xge.h"
#include <stdio.h>
#include <string.h>

static const char g_sPage[] =
	"{"
	"\"xui\":1,"
	"\"tokens\":{\"spacing\":{\"row\":20,\"head\":24},\"colors\":{\"bg\":\"#EEF8FF\",\"line\":\"#86B8D8\",\"text\":\"#1F3A52\",\"sel\":\"#C8E6F8\"}},"
	"\"styles\":{"
	"\"root\":{\"layout\":\"column\",\"gap\":6,\"padding\":8,\"backgroundColor\":\"@colors.bg\"},"
	"\"compact\":{\"height\":28,\"textColor\":\"@colors.text\"},"
	"\"range\":{\"width\":180,\"height\":24},"
	"\"treeStyle\":{\"itemHeight\":\"@spacing.row\",\"indent\":14,\"backgroundColor\":\"#F7FCFF\",\"selectedColor\":\"@colors.sel\",\"textColor\":\"@colors.text\"},"
	"\"tableStyle\":{\"headerHeight\":\"@spacing.head\",\"rowHeight\":\"@spacing.row\",\"backgroundColor\":\"#F7FCFF\",\"headerColor\":\"#D8ECF8\",\"selectedColor\":\"@colors.sel\",\"gridColor\":\"@colors.line\",\"textColor\":\"@colors.text\"},"
	"\"gridStyle\":{\"rowHeight\":\"@spacing.row\",\"nameWidth\":100,\"backgroundColor\":\"#F7FCFF\",\"selectedColor\":\"@colors.sel\",\"gridColor\":\"@colors.line\",\"textColor\":\"@colors.text\"}"
	"},"
	"\"tree\":{\"type\":\"column\",\"id\":\"root\",\"style\":\"root\",\"width\":640,\"height\":720,\"children\":["
	"{\"type\":\"label\",\"id\":\"title\",\"style\":\"compact\",\"text\":\"XSON Controls\"},"
	"{\"type\":\"button\",\"id\":\"button\",\"style\":\"compact\",\"text\":\"Apply\",\"tooltip\":{\"text\":\"Apply changes\",\"anchor\":\"right\",\"offsetX\":4,\"offsetY\":2,\"delay\":0}},"
	"{\"type\":\"input\",\"id\":\"input\",\"style\":\"compact\",\"value\":\"query\",\"placeholder\":\"Search\"},"
	"{\"type\":\"numericInput\",\"id\":\"num\",\"style\":\"compact\",\"min\":0,\"max\":10,\"step\":0.5,\"value\":4.5},"
	"{\"type\":\"checkbox\",\"id\":\"check\",\"style\":\"compact\",\"text\":\"Snap\",\"checked\":true},"
	"{\"type\":\"radio\",\"id\":\"radio\",\"style\":\"compact\",\"text\":\"Detailed\",\"checked\":true,\"value\":2},"
	"{\"type\":\"toggle\",\"id\":\"toggle\",\"style\":\"compact\",\"checked\":true,\"uncheckedText\":\"OFF\",\"checkedText\":\"ON\"},"
	"{\"type\":\"slider\",\"id\":\"slider\",\"style\":\"range\",\"min\":0,\"max\":100,\"value\":64},"
	"{\"type\":\"progress\",\"id\":\"progress\",\"style\":\"range\",\"min\":0,\"max\":100,\"value\":72,\"text\":\"72%\"},"
	"{\"type\":\"colorPicker\",\"id\":\"color\",\"width\":260,\"height\":132,\"value\":\"#2E7CD6FF\",\"palette\":[\"#2E7CD6FF\",\"#34A853FF\",\"#EA4335FF\"]},"
	"{\"type\":\"datePicker\",\"id\":\"date\",\"width\":220,\"height\":28,\"mode\":\"date\",\"value\":\"2024-02-29\",\"min\":\"2024-02-01\",\"max\":\"2024-12-31\"},"
	"{\"type\":\"tabs\",\"id\":\"tabs\",\"width\":220,\"height\":30,\"selected\":1,\"items\":[\"Scene\",\"Assets\",\"Log\"]},"
	"{\"type\":\"toolbar\",\"id\":\"toolbar\",\"width\":260,\"height\":30,\"items\":[{\"text\":\"New\"},{\"text\":\"Pin\",\"type\":\"toggle\",\"checked\":true},{\"type\":\"separator\"},{\"text\":\"Run\",\"enabled\":false}]},"
	"{\"type\":\"statusBar\",\"id\":\"status\",\"width\":360,\"height\":26,\"items\":[{\"section\":\"left\",\"text\":\"Ready\",\"width\":70,\"clickable\":true},{\"section\":\"left\",\"type\":\"progress\",\"min\":0,\"max\":100,\"value\":45,\"width\":110},{\"section\":\"center\",\"text\":\"Ln 12\",\"width\":80},{\"section\":\"right\",\"text\":\"UTF-8\",\"width\":70,\"enabled\":false}]},"
	"{\"type\":\"treeView\",\"id\":\"treeView\",\"style\":\"treeStyle\",\"width\":220,\"height\":90,\"selected\":30,\"nodes\":[{\"id\":10,\"text\":\"Project\",\"expanded\":true},{\"id\":20,\"parent\":10,\"text\":\"src\"},{\"id\":30,\"parent\":10,\"text\":\"main.c\"}]},"
	"{\"type\":\"tableView\",\"id\":\"tableView\",\"style\":\"tableStyle\",\"width\":280,\"height\":116,\"selected\":1,\"columns\":[{\"id\":1,\"title\":\"Name\",\"width\":120},{\"id\":2,\"title\":\"Value\",\"width\":80,\"align\":\"right\"}],\"rows\":[[\"A\",1],[\"B\",2],[\"C\",3]]},"
	"{\"type\":\"propertyGrid\",\"id\":\"props\",\"style\":\"gridStyle\",\"width\":260,\"height\":116,\"selected\":1,\"categories\":[{\"name\":\"General\",\"expanded\":true,\"properties\":[{\"name\":\"Name\",\"value\":\"Player\",\"editor\":\"text\"},{\"name\":\"Count\",\"value\":\"12\",\"editor\":\"number\",\"changed\":true}]}]},"
	"{\"type\":\"breadcrumb\",\"id\":\"path\",\"width\":260,\"height\":28,\"selected\":1,\"segments\":[\"Home\",{\"text\":\"Assets\",\"id\":20},\"Texture\"]},"
	"{\"type\":\"accordion\",\"id\":\"acc\",\"width\":260,\"height\":116,\"mode\":\"single\",\"selected\":1,\"sections\":[{\"title\":\"General\",\"text\":\"Alpha\",\"expanded\":true},{\"title\":\"Advanced\",\"text\":\"Beta\",\"expanded\":true}]},"
	"{\"type\":\"toast\",\"id\":\"toast\",\"width\":320,\"height\":130,\"items\":[{\"type\":\"info\",\"title\":\"Info\",\"message\":\"Queued\",\"duration\":8},{\"type\":\"success\",\"title\":\"Saved\",\"text\":\"Done\",\"duration\":8}]}"
	"]}"
	"}";

int main(void)
{
	static xge_xui_context_t tXui;
	static xge_xui_page_t tPage;
	xge_xui_widget pButtonWidget;
	int bLoadOK;
	int bBasicOK;
	int bChoiceOK;
	int bRangeOK;
	int bPickerOK;
	int bBarsOK;
	int bDataOK;
	int bExperienceOK;
	int bTooltipOK;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tPage, 0, sizeof(tPage));
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		printf("xui-xson-controls-lab init failed\n");
		return 1;
	}
	bLoadOK = xgeXuiPageLoadMemory(&tXui, g_sPage, (int)strlen(g_sPage), NULL, &tPage) == XGE_OK;
	if ( !bLoadOK ) {
		printf("xui-xson-controls-lab load failed: %s\n", xgeXuiPageGetError(&tPage));
		xgeXuiUnit(&tXui);
		return 2;
	}

	pButtonWidget = xgeXuiPageFind(&tPage, "button");
	bTooltipOK = (pButtonWidget != NULL) &&
		(xgeXuiWidgetGetTooltip(pButtonWidget)->iType == XGE_XUI_TOOLTIP_TEXT) &&
		(strcmp(xgeXuiWidgetGetTooltip(pButtonWidget)->sText, "Apply changes") == 0) &&
		(xgeXuiWidgetGetTooltip(pButtonWidget)->iAnchor == XGE_XUI_TOOLTIP_ANCHOR_WIDGET_RIGHT) &&
		(xgeXuiWidgetGetTooltip(pButtonWidget)->fDelay == 0.0f);
	bBasicOK = (tPage.iLabelCount == 1) && (tPage.iButtonCount == 1) && (tPage.iInputCount == 1) && (tPage.iNumericInputCount == 1) && bTooltipOK &&
		(tPage.arrNumericInput[0].fValue == 4.5f);
	bChoiceOK = (tPage.iCheckBoxCount == 1) && (tPage.iRadioCount == 1) && (tPage.iToggleCount == 1) &&
		(xgeXuiCheckBoxGetChecked(&tPage.arrCheckBox[0]) == 1) && (xgeXuiRadioGetChecked(&tPage.arrRadio[0]) == 1) && (xgeXuiToggleGetChecked(&tPage.arrToggle[0]) == 1);
	bRangeOK = (tPage.iSliderCount == 1) && (tPage.iProgressCount == 1) && (tPage.iTabsCount == 1) &&
		(tPage.arrSlider[0].fValue == 64.0f) && (tPage.arrProgress[0].fValue == 72.0f) && (tPage.arrTabs[0].iSelected == 1);
	bPickerOK = (tPage.iColorPickerCount == 1) && (tPage.iDatePickerCount == 1) &&
		(tPage.arrColorPicker[0].iColor == XGE_COLOR_RGBA(0x2E, 0x7C, 0xD6, 0xFF)) &&
		(tPage.arrColorPicker[0].iPaletteCount == 3) &&
		(xgeXuiDatePickerGetMode(&tPage.arrDatePicker[0]) == XGE_XUI_DATE_PICKER_MODE_DATE) &&
		xgeXuiDatePickerHasValue(&tPage.arrDatePicker[0]) &&
		tPage.arrDatePicker[0].bHasMin &&
		tPage.arrDatePicker[0].bHasMax;
	bBarsOK = (tPage.iToolbarCount == 1) && (tPage.iStatusBarCount == 1) &&
		(tPage.arrToolbar[0].iItemCount == 4) && (tPage.arrStatusBar[0].iItemCount == 4);
	bDataOK = (tPage.iTreeViewCount == 1) && (tPage.iTableViewCount == 1) && (tPage.iPropertyGridCount == 1) &&
		(xgeXuiTreeViewGetSelected(&tPage.arrTreeView[0]) == 30);
	bExperienceOK = (tPage.iBreadcrumbCount == 1) && (tPage.iAccordionCount == 1) && (tPage.iToastCount == 1) &&
		(xgeXuiBreadcrumbGetSegmentCount(&tPage.arrBreadcrumb[0]) == 3) &&
		(xgeXuiAccordionGetSectionCount(&tPage.arrAccordion[0]) == 2) &&
		(xgeXuiToastGetCount(&tPage.arrToast[0]) == 2);

	printf(
		"xui-xson-controls-lab final-summary load=%d basic=%d choice=%d range=%d pickers=%d bars=%d data=%d experience=%d tooltip=%d index=%d\n",
		bLoadOK,
		bBasicOK,
		bChoiceOK,
		bRangeOK,
		bPickerOK,
		bBarsOK,
		bDataOK,
		bExperienceOK,
		bTooltipOK,
		tPage.iIndexCount);

	xgeXuiPageUnload(&tPage);
	xgeXuiUnit(&tXui);
	return (bLoadOK && bBasicOK && bChoiceOK && bRangeOK && bPickerOK && bBarsOK && bDataOK && bExperienceOK) ? 0 : 3;
}
