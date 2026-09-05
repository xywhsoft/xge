#ifndef MAPEDIT_XRT_H
#define MAPEDIT_XRT_H

/* Project-local adapters over the public XRT API exposed by xge.h. */
#include "xge.h"

#include <string.h>

static inline xstrview mapeditXrtText(const char* sText, size_t iSize)
{
	xstrview tView;
	tView.Data = sText != NULL ? sText : "";
	tView.Size = (sText != NULL && iSize == 0u) ? strlen(sText) : iSize;
	return tView;
}

static inline xvaluetype xuiXrtValueType(const xvalue* pValue)
{
	return pValue != NULL ? xrtValueType(pValue) : XVALUE_NULL;
}

static inline const char* xuiXrtValueGetText(const xvalue* pValue, ...)
{
	xstrview tText = { 0 };
	return xrtValueGetString(pValue, &tText) ? tText.Data : NULL;
}

static inline int64 xuiXrtValueGetInt(const xvalue* pValue)
{
	int64 iValue = 0;
	(void)xrtValueGetInt(pValue, &iValue);
	return iValue;
}

static inline double xuiXrtValueGetFloat(const xvalue* pValue)
{
	double fValue = 0.0;
	int64 iValue = 0;
	if ( !xrtValueGetFloat(pValue, &fValue) && xrtValueGetInt(pValue, &iValue) ) fValue = (double)iValue;
	return fValue;
}

static inline bool xuiXrtValueGetBool(const xvalue* pValue)
{
	bool bValue = false;
	(void)xrtValueGetBool(pValue, &bValue);
	return bValue;
}

static inline xvalue* xuiXrtValueObjectGet(const xvalue* pObject, const char* sKey, size_t iSize)
{
	return xrtValueObjectGet(pObject, mapeditXrtText(sKey, iSize));
}

static inline bool xuiXrtValueObjectSetTake(xvalue* pObject, const char* sKey, size_t iSize, xvalue* pValue, ...)
{
	xvalue* pTake = pValue;
	return pTake != NULL && xrtValueObjectSetTake(pObject, mapeditXrtText(sKey, iSize), &pTake);
}

static inline bool xuiXrtValueObjectSetText(xvalue* pObject, const char* sKey, size_t iKeySize,
	const char* sValue, size_t iValueSize, ...)
{
	xvalue* pValue = xrtValueString(mapeditXrtText(sValue, iValueSize));
	if ( pValue == NULL ) return false;
	if ( xuiXrtValueObjectSetTake(pObject, sKey, iKeySize, pValue) ) return true;
	xrtValueRelease(pValue);
	return false;
}

static inline bool xuiXrtValueObjectSetInt(xvalue* pObject, const char* sKey, size_t iKeySize, int64 iValue)
{
	xvalue* pValue = xrtValueInt(iValue);
	if ( pValue == NULL ) return false;
	if ( xuiXrtValueObjectSetTake(pObject, sKey, iKeySize, pValue) ) return true;
	xrtValueRelease(pValue);
	return false;
}

static inline bool xuiXrtValueObjectRemove(xvalue* pObject, const char* sKey, size_t iSize)
{
	return xrtValueObjectRemove(pObject, mapeditXrtText(sKey, iSize));
}

static inline bool xuiXrtValueArrayAppendTake(xvalue* pArray, xvalue* pValue, ...)
{
	xvalue* pTake = pValue;
	return pTake != NULL && xrtValueArrayAppendTake(pArray, &pTake);
}

static inline bool xuiXrtValueArrayAppendInt(xvalue* pArray, int64 iValue)
{
	xvalue* pValue = xrtValueInt(iValue);
	if ( pValue == NULL ) return false;
	if ( xuiXrtValueArrayAppendTake(pArray, pValue) ) return true;
	xrtValueRelease(pValue);
	return false;
}

static inline xvalue* xuiXrtValueArrayGet(const xvalue* pArray, size_t iIndex)
{
	return xrtValueArrayGet(pArray, iIndex);
}

#endif
