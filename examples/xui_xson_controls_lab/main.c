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
	"{\"type\":\"button\",\"id\":\"button\",\"style\":\"compact\",\"text\":\"Apply\"},"
	"{\"type\":\"input\",\"id\":\"input\",\"style\":\"compact\",\"value\":\"query\",\"placeholder\":\"Search\"},"
	"{\"type\":\"numericInput\",\"id\":\"num\",\"style\":\"compact\",\"min\":0,\"max\":10,\"step\":0.5,\"value\":4.5},"
	"{\"type\":\"checkbox\",\"id\":\"check\",\"style\":\"compact\",\"text\":\"Snap\",\"checked\":true},"
	"{\"type\":\"radio\",\"id\":\"radio\",\"style\":\"compact\",\"text\":\"Detailed\",\"checked\":true,\"value\":2},"
	"{\"type\":\"switch\",\"id\":\"switch\",\"style\":\"compact\",\"text\":\"Preview\",\"checked\":true},"
	"{\"type\":\"slider\",\"id\":\"slider\",\"style\":\"range\",\"min\":0,\"max\":100,\"value\":64},"
	"{\"type\":\"progress\",\"id\":\"progress\",\"style\":\"range\",\"min\":0,\"max\":100,\"value\":72,\"text\":\"72%\"},"
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
	xge_xui_context_t tXui;
	xge_xui_page_t tPage;
	xge_xui_table_view pTable;
	int bLoadOK;
	int bBasicOK;
	int bChoiceOK;
	int bRangeOK;
	int bBarsOK;
	int bDataOK;
	int bExperienceOK;

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

	bBasicOK = (tPage.iLabelCount == 1) && (tPage.iButtonCount == 1) && (tPage.iInputCount == 1) && (tPage.iNumericInputCount == 1) &&
		(tPage.arrNumericInput[0].fValue == 4.5f);
	bChoiceOK = (tPage.iCheckBoxCount == 1) && (tPage.iRadioCount == 1) && (tPage.iSwitchCount == 1) &&
		(tPage.arrCheckBox[0].bChecked == 1) && (tPage.arrRadio[0].bChecked == 1) && (tPage.arrSwitch[0].bChecked == 1);
	bRangeOK = (tPage.iSliderCount == 1) && (tPage.iProgressCount == 1) && (tPage.iTabsCount == 1) &&
		(tPage.arrSlider[0].fValue == 64.0f) && (tPage.arrProgress[0].fValue == 72.0f) && (tPage.arrTabs[0].iSelected == 1);
	bBarsOK = (tPage.iToolbarCount == 1) && (tPage.iStatusBarCount == 1) &&
		(tPage.arrToolbar[0].iItemCount == 4) && (tPage.arrStatusBar[0].iItemCount == 4);
	pTable = (tPage.iTableViewCount > 0) ? &tPage.arrTableView[0] : NULL;
	bDataOK = (tPage.iTreeViewCount == 1) && (tPage.iTableViewCount == 1) && (tPage.iPropertyGridCount == 1) &&
		(xgeXuiTreeViewGetSelected(&tPage.arrTreeView[0]) == 30) &&
		(pTable != NULL) && (xgeXuiTableViewGetRowCount(pTable) == 3) && (tPage.arrTableViewAdapter[0] != NULL) &&
		(strcmp(tPage.arrTableViewAdapter[0]->arrCell[1][1], "2") == 0) &&
		(xgeXuiPropertyGridGetVisibleCount(&tPage.arrPropertyGrid[0]) == 3);
	bExperienceOK = (tPage.iBreadcrumbCount == 1) && (tPage.iAccordionCount == 1) && (tPage.iToastCount == 1) &&
		(xgeXuiBreadcrumbGetSegmentCount(&tPage.arrBreadcrumb[0]) == 3) &&
		(xgeXuiAccordionGetSectionCount(&tPage.arrAccordion[0]) == 2) &&
		(xgeXuiToastGetCount(&tPage.arrToast[0]) == 2);

	printf(
		"xui-xson-controls-lab final-summary load=%d basic=%d choice=%d range=%d bars=%d data=%d experience=%d index=%d\n",
		bLoadOK,
		bBasicOK,
		bChoiceOK,
		bRangeOK,
		bBarsOK,
		bDataOK,
		bExperienceOK,
		tPage.iIndexCount);

	xgeXuiPageUnload(&tPage);
	xgeXuiUnit(&tXui);
	return (bLoadOK && bBasicOK && bChoiceOK && bRangeOK && bBarsOK && bDataOK && bExperienceOK) ? 0 : 3;
}
