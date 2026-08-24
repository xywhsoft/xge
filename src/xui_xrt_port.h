#ifndef XUI_XRT_PORT_H
#define XUI_XRT_PORT_H

#include "../xui.h"

#include <string.h>

static inline xbytesview xuiXrtBytes(const void* pData, size_t iSize)
{
	xbytesview tView = { (const uint8*)pData, iSize };
	return tView;
}

static inline xstrview xuiXrtText(const char* sText, size_t iSize)
{
	xstrview tView;
	tView.Data = sText;
	tView.Size = (sText != NULL && iSize == 0u) ? strlen(sText) : iSize;
	return tView;
}

static inline bool xuiXrtArrayInit(xarray* pArray, size_t iItemSize, ...)
{
	return xrtArrayInit(pArray, iItemSize);
}

/* XUI's existing model indexes are one-based. Keep that local model stable. */
static inline size_t xuiXrtArrayAppendSpace(xarray* pArray, size_t iCount)
{
	size_t iPosition = pArray != NULL ? pArray->Count + 1u : 0u;
	return (pArray != NULL && xrtArrayAdd(pArray, iCount) != NULL) ? iPosition : 0u;
}

static inline void* xuiXrtArrayGet(xarray* pArray, size_t iPosition)
{
	return (pArray != NULL && iPosition > 0u) ? xrtArrayGet(pArray, iPosition - 1u) : NULL;
}

static inline bool xuiXrtArrayRemove(xarray* pArray, size_t iPosition, size_t iCount)
{
	return iPosition > 0u && xrtArrayRemove(pArray, iPosition - 1u, iCount);
}

static inline xbytesview xuiXrtKey(const void* pKey, size_t iSize)
{
	return xuiXrtBytes(pKey, (pKey != NULL && iSize == 0u) ? strlen((const char*)pKey) : iSize);
}

static inline bool xuiXrtMapInit(xmap* pMap, size_t iValueSize, ...)
{
	return xrtMapInit(pMap, iValueSize);
}

static inline void* xuiXrtMapGet(xmap* pMap, const void* pKey, size_t iSize)
{
	return xrtMapGet(pMap, xuiXrtKey(pKey, iSize));
}

static inline void* xuiXrtMapGetOrAdd(xmap* pMap, const void* pKey, size_t iSize, bool* pNew)
{
	return xrtMapGetOrAdd(pMap, xuiXrtKey(pKey, iSize), pNew);
}

static inline void* xuiXrtMapGetPtr(xmap* pMap, const void* pKey, size_t iSize)
{
	return xrtMapGetPtr(pMap, xuiXrtKey(pKey, iSize));
}

static inline bool xuiXrtMapSetPtr(xmap* pMap, const void* pKey, size_t iSize, void* pValue, ...)
{
	return xrtMapSetPtr(pMap, xuiXrtKey(pKey, iSize), pValue);
}

static inline bool xuiXrtMapRemove(xmap* pMap, const void* pKey, size_t iSize)
{
	return xrtMapRemove(pMap, xuiXrtKey(pKey, iSize));
}

static inline xvaluetype xuiXrtValueType(const xvalue* pValue)
{
	return pValue != NULL ? xrtValueType(pValue) : XVALUE_NULL;
}

static inline xvalue* xuiXrtValueText(const char* sText, size_t iSize)
{
	return xrtValueString(xuiXrtText(sText, iSize));
}

static inline xvalue* xuiXrtValueCreateText(const void* pText, size_t iSize, ...)
{
	return xuiXrtValueText((const char*)pText, iSize);
}

static inline size_t xuiXrtValueSize(const xvalue* pValue)
{
	xstrview tText = { 0 };
	return xrtValueGetString(pValue, &tText) ? tText.Size : xrtValueCount(pValue);
}

static inline bool xuiXrtValueIsArray(const xvalue* pValue) { return xrtValueIs(pValue, XVALUE_ARRAY); }
static inline bool xuiXrtValueIsObject(const xvalue* pValue) { return xrtValueIs(pValue, XVALUE_OBJECT); }
static inline bool xuiXrtValueIsText(const xvalue* pValue) { return xrtValueIs(pValue, XVALUE_STRING); }
static inline bool xuiXrtValueIsBool(const xvalue* pValue) { return xrtValueIs(pValue, XVALUE_BOOL); }
static inline bool xuiXrtValueIsInt(const xvalue* pValue) { return xrtValueIs(pValue, XVALUE_INT); }
static inline bool xuiXrtValueIsNull(const xvalue* pValue) { return pValue == NULL || xrtValueIs(pValue, XVALUE_NULL); }
static inline bool xuiXrtValueIsNumber(const xvalue* pValue) { return xrtValueIsNumber(pValue); }

static inline const char* xuiXrtValueGetText(const xvalue* pValue, ...)
{
	xstrview tText = { 0 };
	if ( !xrtValueGetString(pValue, &tText) ) return NULL;
	return tText.Data;
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
	if ( !xrtValueGetFloat(pValue, &fValue) ) {
		int64 iValue = 0;
		if ( xrtValueGetInt(pValue, &iValue) ) fValue = (double)iValue;
	}
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
	return xrtValueObjectGet(pObject, xuiXrtText(sKey, iSize));
}

static inline const char* xuiXrtValueObjectGetText(const xvalue* pObject, const char* sKey, size_t iSize)
{
	return xuiXrtValueGetText(xuiXrtValueObjectGet(pObject, sKey, iSize), NULL);
}

static inline int64 xuiXrtValueObjectGetInt(const xvalue* pObject, const char* sKey, size_t iSize)
{
	return xuiXrtValueGetInt(xuiXrtValueObjectGet(pObject, sKey, iSize));
}

static inline double xuiXrtValueObjectGetFloat(const xvalue* pObject, const char* sKey, size_t iSize)
{
	return xuiXrtValueGetFloat(xuiXrtValueObjectGet(pObject, sKey, iSize));
}

static inline bool xuiXrtValueObjectGetBool(const xvalue* pObject, const char* sKey, size_t iSize)
{
	return xuiXrtValueGetBool(xuiXrtValueObjectGet(pObject, sKey, iSize));
}

static inline bool xuiXrtValueObjectSetTake(xvalue* pObject, const char* sKey, size_t iSize, xvalue* pValue, ...)
{
	xvalue* pTake = pValue;
	return pTake != NULL && xrtValueObjectSetTake(pObject, xuiXrtText(sKey, iSize), &pTake);
}

static inline bool xuiXrtValueObjectSetText(xvalue* pObject, const char* sKey, size_t iKeySize,
	const char* sValue, size_t iValueSize, ...)
{
	xvalue* pValue = xuiXrtValueText(sValue, iValueSize);
	if ( pValue == NULL ) return false;
	if ( !xuiXrtValueObjectSetTake(pObject, sKey, iKeySize, pValue) ) {
		xrtValueRelease(pValue);
		return false;
	}
	return true;
}

static inline bool xuiXrtValueObjectSetInt(xvalue* pObject, const char* sKey, size_t iKeySize, int64 iValue)
{
	xvalue* pValue = xrtValueInt(iValue);
	if ( pValue == NULL ) return false;
	if ( xuiXrtValueObjectSetTake(pObject, sKey, iKeySize, pValue) ) return true;
	xrtValueRelease(pValue);
	return false;
}

static inline bool xuiXrtValueObjectSetFloat(xvalue* pObject, const char* sKey, size_t iKeySize, double fValue)
{
	xvalue* pValue = xrtValueFloat(fValue);
	if ( pValue == NULL ) return false;
	if ( xuiXrtValueObjectSetTake(pObject, sKey, iKeySize, pValue) ) return true;
	xrtValueRelease(pValue);
	return false;
}

static inline bool xuiXrtValueObjectSetBool(xvalue* pObject, const char* sKey, size_t iKeySize, bool bValue)
{
	xvalue* pValue = xrtValueBool(bValue);
	if ( pValue == NULL ) return false;
	if ( xuiXrtValueObjectSetTake(pObject, sKey, iKeySize, pValue) ) return true;
	xrtValueRelease(pValue);
	return false;
}

static inline bool xuiXrtValueObjectRemove(xvalue* pObject, const char* sKey, size_t iSize)
{
	return xrtValueObjectRemove(pObject, xuiXrtText(sKey, iSize));
}

static inline bool xuiXrtValueObjectMerge(xvalue* pTarget, const xvalue* pSource, ...)
{
	return xrtValueObjectMerge(pTarget, pSource, XVALUE_MERGE_REPLACE);
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

static inline bool xuiXrtValueArrayAppendFloat(xvalue* pArray, double fValue)
{
	xvalue* pValue = xrtValueFloat(fValue);
	if ( pValue == NULL ) return false;
	if ( xuiXrtValueArrayAppendTake(pArray, pValue) ) return true;
	xrtValueRelease(pValue);
	return false;
}

static inline bool xuiXrtValueArrayAppendText(xvalue* pArray, const char* sText, size_t iSize, ...)
{
	xvalue* pValue = xuiXrtValueText(sText, iSize);
	if ( pValue == NULL ) return false;
	if ( xuiXrtValueArrayAppendTake(pArray, pValue) ) return true;
	xrtValueRelease(pValue);
	return false;
}

static inline xvalue* xuiXrtValueArrayGet(const xvalue* pArray, size_t iIndex)
{
	return xrtValueArrayGet(pArray, iIndex);
}

typedef struct xui_xrt_regex_t {
	xregex* pRegex;
	xregexmatcher* pMatcher;
} xui_xrt_regex_t;

static inline xui_xrt_regex_t* xuiXrtRegexCreate(const char* sPattern, size_t iSize, uint32 iFlags)
{
	xregexconfig tConfig;
	xui_xrt_regex_t* pResult;
	xrtRegexConfigInit(&tConfig);
	if ( (iFlags & UINT32_C(1)) != 0u ) tConfig.Flags |= XREGEX_IGNORE_CASE;
	if ( (iFlags & UINT32_C(2)) != 0u ) tConfig.Flags |= XREGEX_MULTILINE;
	if ( (iFlags & UINT32_C(4)) != 0u ) tConfig.Flags |= XREGEX_DOT_ALL;
	pResult = (xui_xrt_regex_t*)xrtCalloc(1u, sizeof(*pResult));
	if ( pResult == NULL ) return NULL;
	pResult->pRegex = xrtRegexCompileConfig(xuiXrtText(sPattern, iSize), &tConfig);
	if ( pResult->pRegex != NULL ) pResult->pMatcher = xrtRegexMatcherCreate(pResult->pRegex);
	if ( pResult->pMatcher == NULL ) {
		if ( pResult->pRegex != NULL ) xrtRegexRelease(pResult->pRegex);
		xrtFree(pResult);
		return NULL;
	}
	return pResult;
}

static inline void xuiXrtRegexDestroy(xui_xrt_regex_t* pRegex)
{
	if ( pRegex == NULL ) return;
	xrtRegexMatcherFree(pRegex->pMatcher);
	xrtRegexRelease(pRegex->pRegex);
	xrtFree(pRegex);
}

static inline xregexresult xuiXrtRegexFindAt(xui_xrt_regex_t* pRegex, const char* sText,
	size_t iSize, size_t iStart, xregexspan* pSpan)
{
	xregexcapture tCapture;
	xregexresult iResult;
	if ( pRegex == NULL || pSpan == NULL ) return XREGEX_ERROR;
	iResult = xrtRegexMatcherFind(pRegex->pMatcher, xuiXrtText(sText, iSize), iStart);
	if ( iResult == XREGEX_MATCH && xrtRegexMatcherCapture(pRegex->pMatcher, 0u, &tCapture) )
		*pSpan = tCapture.Span;
	return iResult;
}

static inline xtime xuiXrtDateSerial(int64 iYear, int iMonth, int iDay)
{
	xtime tValue = 0;
	(void)xrtDate(iYear, iMonth, iDay, &tValue);
	return tValue;
}

static inline void xuiXrtDecodeSerial(xtime tValue, int64* pYear, int* pMonth, int* pDay,
	int* pHour, int* pMinute, int* pSecond, int* pWeekday, int* pYearDay)
{
	xdatetime tParts;
	if ( !xrtTimeSplit(tValue, &tParts) ) memset(&tParts, 0, sizeof(tParts));
	if ( pYear != NULL ) *pYear = (int)tParts.Year;
	if ( pMonth != NULL ) *pMonth = tParts.Month;
	if ( pDay != NULL ) *pDay = tParts.Day;
	if ( pHour != NULL ) *pHour = tParts.Hour;
	if ( pMinute != NULL ) *pMinute = tParts.Minute;
	if ( pSecond != NULL ) *pSecond = tParts.Second;
	if ( pWeekday != NULL ) *pWeekday = tParts.Weekday;
	if ( pYearDay != NULL ) *pYearDay = tParts.YearDay;
}

static inline xtime xuiXrtDateAddMonth(int iInterval, int64 iValue, xtime tValue)
{
	xtime tResult = tValue;
	(void)iInterval;
	(void)xrtTimeAdd(tValue, iValue, XTIME_UNIT_MONTH, &tResult);
	return tResult;
}

static inline xtime xuiXrtFirstDayOfMonth(xtime tValue)
{
	xdatetime tParts;
	xtime tResult = 0;
	if ( xrtTimeSplit(tValue, &tParts) ) (void)xrtDate(tParts.Year, tParts.Month, 1, &tResult);
	return tResult;
}

static inline xtime xuiXrtTimeParse(const char* sText, const char* sFormat)
{
	xtime tResult = 0;
	(void)xrtTimeParse(xuiXrtText(sText, 0u), xuiXrtText(sFormat, 0u), &tResult);
	return tResult;
}

#endif
