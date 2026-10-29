#ifndef XUI_CODE_PROVIDER_INTERNAL_H
#define XUI_CODE_PROVIDER_INTERNAL_H

#include "xui.h"

#include <string.h>

static inline int xuiInternalCodeProviderHoverValid(
	const xui_code_hover_t* pHover)
{
	return pHover != NULL && pHover->iSize >= sizeof(*pHover) &&
		pHover->tRange.iStart >= 0 &&
		pHover->tRange.iEnd >= pHover->tRange.iStart &&
		pHover->sText != NULL;
}

static inline int xuiInternalCodeProviderSignatureValid(
	const xui_code_signature_help_t* pHelp)
{
	size_t iLabelSize;
	int i;

	if ( pHelp == NULL || pHelp->iSize < sizeof(*pHelp) ||
	     pHelp->sLabel == NULL || pHelp->iParameterCount < 0 ||
	     (pHelp->iParameterCount > 0 && pHelp->pParameters == NULL) ||
	     pHelp->iActiveParameter < -1 ||
	     pHelp->iActiveParameter >= pHelp->iParameterCount ) {
		return 0;
	}
	iLabelSize = strlen(pHelp->sLabel);
	for ( i = 0; i < pHelp->iParameterCount; i++ ) {
		const xui_code_signature_parameter_t* pParameter = &pHelp->pParameters[i];
		if ( pParameter->iSize < sizeof(*pParameter) ||
		     pParameter->tLabelRange.iStart < 0 ||
		     pParameter->tLabelRange.iEnd < pParameter->tLabelRange.iStart ||
		     (size_t)pParameter->tLabelRange.iEnd > iLabelSize ) {
			return 0;
		}
	}
	return 1;
}

#endif
