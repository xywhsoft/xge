#include "xui_config.h"
#if XUI_ENABLE_TABLE_GRID
#include "xui_internal.h"
#include <string.h>
#include <stdint.h>

static int __xuiConfigText(xstrview text,xstrview* result)
{
    if(text.Size==SIZE_MAX || (text.Size && !text.Data) || (text.Size && memchr(text.Data,0,text.Size)))return XUI_ERROR_INVALID_ARGUMENT;
    char* data=xrtMalloc(text.Size+1);if(!data)return XUI_ERROR_OUT_OF_MEMORY;
    if(text.Size)memcpy(data,text.Data,text.Size);
    data[text.Size]=0;
    *result=(xstrview){data,text.Size};return XUI_OK;
}
XUI_API void xuiTableGridEditorConfigOutputInit(xui_table_grid_editor_config_output_t* output)
{
    if(!output)return;
    memset(output,0,sizeof(*output));output->iEnumSelected=-1;output->iPrecision=-1;
    output->iDateMode=XUI_DATE_PICKER_MODE_DATE;output->bShowSecond=1;
}
XUI_API void xuiTableGridEditorConfigOutputDispose(xui_table_grid_editor_config_output_t* output)
{
    if(!output)return;
    xui_table_grid_editor_config_output_t retired=*output;xuiTableGridEditorConfigOutputInit(output);
    for(int i=0;i<retired._iEnumCount;++i)xrtFree((void*)retired._pEnumItems[i].sText);
    xrtFree(retired._pEnumItems);xrtFree(retired._pPalette);
    xrtFree((void*)retired._tDateFormat.Data);xrtFree((void*)retired._tActionText.Data);
    xrtFree((void*)retired._tFileFilter.Data);xrtFree((void*)retired._tRangeSeparator.Data);
}
XUI_API int xuiTableGridEditorConfigOutputSetEnumCount(xui_table_grid_editor_config_output_t* output,int count)
{
    if(!output || count<0 || count>XUI_COMBOBOX_ITEM_CAPACITY)return XUI_ERROR_INVALID_ARGUMENT;
    int oldCount=output->_iEnumCount;xui_combobox_item_t* old=output->_pEnumItems;
    if(oldCount<0 || oldCount>XUI_COMBOBOX_ITEM_CAPACITY || (oldCount && !old))return XUI_ERROR_INVALID_STATE;
    if(count==oldCount)return XUI_OK;
    xui_combobox_item_t* next=count ? xrtCalloc((size_t)count,sizeof(*next)):NULL;
    if(count && !next)return XUI_ERROR_OUT_OF_MEMORY;
    int keep=count<oldCount ? count:oldCount;
    if(keep)memcpy(next,old,(size_t)keep*sizeof(*next));
    for(int i=keep;i<count;++i)next[i].bEnabled=1;
    output->_pEnumItems=next;output->_iEnumCount=count;
    for(int i=keep;i<oldCount;++i)xrtFree((void*)old[i].sText);
    xrtFree(old);return XUI_OK;
}
XUI_API int xuiTableGridEditorConfigOutputGetEnumCount(const xui_table_grid_editor_config_output_t* output)
{return output ? output->_iEnumCount:0;}
XUI_API int xuiTableGridEditorConfigOutputGetEnumItem(const xui_table_grid_editor_config_output_t* output,int index,xstrview* text,int* value,int* enabled,int* separator,int* icon)
{
    if(!output || index<0 || index>=output->_iEnumCount || !text || !value || !enabled || !separator || !icon)return XUI_ERROR_INVALID_ARGUMENT;
    const xui_combobox_item_t* item=&output->_pEnumItems[index];const char* data=item->sText ? item->sText:"";
    *text=(xstrview){data,strlen(data)};*value=item->iValue;*enabled=item->bEnabled;*separator=item->bSeparator;*icon=item->iIcon;
    return XUI_OK;
}
XUI_API int xuiTableGridEditorConfigOutputSetEnumItem(xui_table_grid_editor_config_output_t* output,int index,xstrview text,int value,int enabled,int separator,int icon)
{
    if(!output || index<0 || index>=output->_iEnumCount)return XUI_ERROR_INVALID_ARGUMENT;
    xstrview copy={0};int status=__xuiConfigText(text,&copy);if(status!=XUI_OK)return status;
    const char* old=output->_pEnumItems[index].sText;
    output->_pEnumItems[index]=(xui_combobox_item_t){.sText=copy.Data,.iValue=value,.bEnabled=enabled!=0,.bSeparator=separator!=0,.iIcon=icon};
    xrtFree((void*)old);return XUI_OK;
}
XUI_API int xuiTableGridEditorConfigOutputSetPalette(xui_table_grid_editor_config_output_t* output,const uint32_t* colors,int count)
{
    if(!output || count<0 || count>XUI_COLOR_PICKER_PALETTE_CAPACITY || (count && !colors))return XUI_ERROR_INVALID_ARGUMENT;
    uint32_t* next=count ? xrtMalloc((size_t)count*sizeof(*next)):NULL;
    if(count && !next)return XUI_ERROR_OUT_OF_MEMORY;
    if(count)memcpy(next,colors,(size_t)count*sizeof(*next));
    uint32_t* old=output->_pPalette;output->_pPalette=next;output->_iPaletteCount=count;xrtFree(old);return XUI_OK;
}
XUI_API int xuiTableGridEditorConfigOutputSetPaletteCount(xui_table_grid_editor_config_output_t* output,int count)
{
    if(!output || count<0 || count>XUI_COLOR_PICKER_PALETTE_CAPACITY)return XUI_ERROR_INVALID_ARGUMENT;
    if(count==output->_iPaletteCount)return XUI_OK;
    uint32_t* next=count ? xrtCalloc((size_t)count,sizeof(*next)):NULL;
    if(count && !next)return XUI_ERROR_OUT_OF_MEMORY;
    int keep=count<output->_iPaletteCount ? count:output->_iPaletteCount;
    if(keep)memcpy(next,output->_pPalette,(size_t)keep*sizeof(*next));
    uint32_t* old=output->_pPalette;output->_pPalette=next;output->_iPaletteCount=count;xrtFree(old);return XUI_OK;
}
XUI_API int xuiTableGridEditorConfigOutputSetPaletteItem(xui_table_grid_editor_config_output_t* output,int index,uint32_t color)
{
    if(!output || index<0 || index>=output->_iPaletteCount)return XUI_ERROR_INVALID_ARGUMENT;
    output->_pPalette[index]=color;return XUI_OK;
}
XUI_API int xuiTableGridEditorConfigOutputGetPaletteItem(const xui_table_grid_editor_config_output_t* output,int index,uint32_t* color)
{
    if(!output || index<0 || index>=output->_iPaletteCount || !color)return XUI_ERROR_INVALID_ARGUMENT;
    *color=output->_pPalette[index];return XUI_OK;
}
XUI_API int xuiTableGridEditorConfigOutputGetPaletteCount(const xui_table_grid_editor_config_output_t* output)
{return output ? output->_iPaletteCount:0;}
XUI_API int xuiTableGridEditorConfigOutputSetDateFormat(xui_table_grid_editor_config_output_t* output,xstrview text)
{
    if(!output)return XUI_ERROR_INVALID_ARGUMENT;
    xstrview copy={0};int status=__xuiConfigText(text,&copy);if(status!=XUI_OK)return status;
    xstrview old=output->_tDateFormat;output->_tDateFormat=copy;xrtFree((void*)old.Data);return XUI_OK;
}
XUI_API xstrview xuiTableGridEditorConfigOutputGetDateFormat(const xui_table_grid_editor_config_output_t* output)
{return output && output->_tDateFormat.Data ? output->_tDateFormat:(xstrview){"",0};}
XUI_API int xuiTableGridEditorConfigOutputSetActionText(xui_table_grid_editor_config_output_t* output,xstrview text)
{
    if(!output)return XUI_ERROR_INVALID_ARGUMENT;
    xstrview copy={0};int status=__xuiConfigText(text,&copy);if(status!=XUI_OK)return status;
    xstrview old=output->_tActionText;output->_tActionText=copy;xrtFree((void*)old.Data);return XUI_OK;
}
XUI_API xstrview xuiTableGridEditorConfigOutputGetActionText(const xui_table_grid_editor_config_output_t* output)
{return output && output->_tActionText.Data ? output->_tActionText:(xstrview){"",0};}
XUI_API int xuiTableGridEditorConfigOutputSetFileFilter(xui_table_grid_editor_config_output_t* output,xstrview text)
{
    if(!output)return XUI_ERROR_INVALID_ARGUMENT;
    xstrview copy={0};int status=__xuiConfigText(text,&copy);if(status!=XUI_OK)return status;
    xstrview old=output->_tFileFilter;output->_tFileFilter=copy;xrtFree((void*)old.Data);return XUI_OK;
}
XUI_API xstrview xuiTableGridEditorConfigOutputGetFileFilter(const xui_table_grid_editor_config_output_t* output)
{return output && output->_tFileFilter.Data ? output->_tFileFilter:(xstrview){"",0};}
XUI_API int xuiTableGridEditorConfigOutputSetRangeSeparator(xui_table_grid_editor_config_output_t* output,xstrview text)
{
    if(!output)return XUI_ERROR_INVALID_ARGUMENT;
    xstrview copy={0};int status=__xuiConfigText(text,&copy);if(status!=XUI_OK)return status;
    xstrview old=output->_tRangeSeparator;output->_tRangeSeparator=copy;xrtFree((void*)old.Data);return XUI_OK;
}
XUI_API xstrview xuiTableGridEditorConfigOutputGetRangeSeparator(const xui_table_grid_editor_config_output_t* output)
{return output && output->_tRangeSeparator.Data ? output->_tRangeSeparator:(xstrview){" - ",3};}
void xuiInternalTableGridConfigProject(const xui_table_grid_editor_config_output_t* output,xui_table_grid_editor_config_t* config)
{
    memset(config,0,sizeof(*config));
    config->iEnumSelected=output->iEnumSelected;
    config->bEnumUseValue=output->bEnumUseValue;
    config->iEnumSelectedValue=output->iEnumSelectedValue;
    config->fMin=output->fMin;
    config->fMax=output->fMax;
    config->fStep=output->fStep;
    config->iPrecision=output->iPrecision;
    config->bNullable=output->bNullable;
    config->iDateMode=output->iDateMode;
    config->bDateModeSet=output->bDateModeSet;
    config->bShowSecond=output->bShowSecond;
    config->tDateMin=output->tDateMin;
    config->tDateMax=output->tDateMax;
    config->bDateHasMin=output->bDateHasMin;
    config->bDateHasMax=output->bDateHasMax;
    config->tDefaultRangeSpan=output->tDefaultRangeSpan;
    config->bAlphaEnabled=output->bAlphaEnabled;
    config->arrEnumItemData=output->_pEnumItems;config->iEnumItemCount=output->_iEnumCount;
    config->arrPalette=output->_pPalette;config->iPaletteCount=output->_iPaletteCount;
    config->sDateFormat=output->_tDateFormat.Data;config->sActionText=output->_tActionText.Data;
    config->sFileFilter=output->_tFileFilter.Data;
    config->sRangeSeparator=output->_tRangeSeparator.Data ? output->_tRangeSeparator.Data:" - ";
}
#endif
