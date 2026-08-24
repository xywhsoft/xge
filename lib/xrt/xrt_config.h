#ifndef XGE_XRT_CONFIG_H
#define XGE_XRT_CONFIG_H

/* XRT is part of xge.dll's public runtime surface. */
#if defined(XGE_BUILD_DLL) || defined(XUI_BUILD_DLL)
	#ifndef XRT_BUILD_SHARED
		#define XRT_BUILD_SHARED
	#endif
#elif defined(XGE_DLL) || defined(XUI_DLL)
	#ifndef XRT_USE_SHARED
		#define XRT_USE_SHARED
	#endif
#endif

/* XGE's shared XRT profile. Keep this list identical in every translation unit. */
#define XRT_MODULE_ARRAY
#define XRT_MODULE_ATOMIC
#define XRT_MODULE_BUFFER
#define XRT_MODULE_ERROR_FORMAT
#define XRT_MODULE_FILE_TEXT
#define XRT_MODULE_FILE_TREE
#define XRT_MODULE_FILE_WALK
#define XRT_MODULE_FILE_WHOLE
#define XRT_MODULE_JSON
#define XRT_MODULE_MAP
#define XRT_MODULE_MUTEX
#define XRT_MODULE_PATH
#define XRT_MODULE_POOL
#define XRT_MODULE_SPIN
#define XRT_MODULE_STRING_FORMAT
#define XRT_MODULE_STRING_GLOB
#define XRT_MODULE_THREAD
#define XRT_MODULE_TIME_TEXT
#define XRT_MODULE_UNICODE_TEXT
#define XRT_MODULE_VALUE
#define XRT_MODULE_VALUE_COLLECTION
#define XRT_MODULE_VALUE_GRAPH
#define XRT_MODULE_XSON

#define XREGEX_MODULE_REGEX_CORE
#define XREGEX_MODULE_REGEX_MATCH

#endif
