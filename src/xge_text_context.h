#ifndef XGE_TEXT_CONTEXT_H
#define XGE_TEXT_CONTEXT_H
#include "../xge.h"
#include "xge_text_input.h"
static inline int __xgeTextContextNormalize(const xge_text_shape_desc_t* input,
    int bytes, xge_text_shape_desc_t* output)
{
    int length;
    if (!__xgeTextLanguageValid(input->sLanguage) || !__xgeTextScriptValid(input->iScript) ||
        !__xgeTextInputRange(input->sText,bytes,input->sContext,input->iContextSize,input->iContextOffset,&length))
        return XGE_ERROR_INVALID_ARGUMENT;
    *output=*input;output->iTextSize=bytes;output->iContextSize=length;
    if (!output->sContext) output->sContext=input->sText;
    return XGE_OK;
}
#endif
