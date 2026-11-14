/* Checked public reads share the actual DPI/error path. No duplicate layout,
 * NUL scan, text snapshot or reference-count layer is introduced here. */
XUI_API int xuiTextLayoutGetState(xui_text_layout layout,xui_vec2_t* size,int* lines,int* truncated)
{
    int status=__xuiTextLayoutEnsureDpi(layout,false);
    if(status!=XUI_OK)return status;
    if(size)*size=layout->tSize;
    if(lines)*lines=layout->iLineCount;
    if(truncated)*truncated=layout->bTruncated;
    return XUI_OK;
}

XUI_API int xuiTextLayoutGetTextView(xui_text_layout layout,xstrview* view)
{
    if(!view)return XUI_ERROR_INVALID_ARGUMENT;
    *view=(xstrview){0};
    int status=__xuiTextLayoutEnsureDpi(layout,false);
    if(status!=XUI_OK)return status;
    if(!layout->sText || layout->iTextSize<0)return XUI_ERROR_INVALID_STATE;
    view->Data=layout->sText;
    view->Size=(size_t)layout->iTextSize;
    return XUI_OK;
}
