/*
 * xanim.h - XANIM animation file format definition
 *
 * Private binary animation format for AnimForge editor.
 * This header defines all on-disk structures, constants, and the
 * runtime player API. The format is designed for compact storage
 * and fast sequential parsing at load time.
 *
 * File layout:
 *   [Header 64 bytes]
 *   [TOC entries]
 *   [Meta Section]
 *   [Resource Section]
 *   [Symbol Section]
 *   [Timeline Section]
 *   [Curve Section]
 *   [Action Section]
 *   [String Table]
 *   [Alignment padding]
 */

#ifndef XANIM_H
#define XANIM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/* Version                                                            */
/* ------------------------------------------------------------------ */

#define XANIM_VERSION_MAJOR  1
#define XANIM_VERSION_MINOR  0

/* ------------------------------------------------------------------ */
/* Magic and header                                                   */
/* ------------------------------------------------------------------ */

#define XANIM_MAGIC          0x4D4E4158u  /* 'XANM' little-endian */
#define XANIM_HEADER_SIZE    64

/* Header flags */
#define XANIM_FLAG_LOOP        0x0001u
#define XANIM_FLAG_HAS_ACTIONS 0x0002u
#define XANIM_FLAG_COMPRESSED  0x0004u
#define XANIM_FLAG_PREMULT     0x0008u

typedef struct xanim_header_t {
	uint32_t iMagic;          /* XANIM_MAGIC */
	uint16_t iVersion;        /* format version, initial = 1 */
	uint16_t iHeaderSize;     /* = XANIM_HEADER_SIZE (64) */
	uint32_t iFlags;          /* XANIM_FLAG_* */
	float    fFrameRate;      /* frames per second (12/24/30/60) */
	uint32_t iFrameCount;     /* total frame count */
	float    fStageWidth;     /* stage width in pixels */
	float    fStageHeight;    /* stage height in pixels */
	uint32_t iSymbolCount;    /* number of symbols */
	uint32_t iLayerCount;     /* number of layers (main timeline) */
	uint32_t iResourceCount;  /* number of embedded resources */
	uint32_t iTocOffset;      /* byte offset to TOC array */
	uint32_t iTocCount;       /* number of TOC entries */
	uint32_t iCrc32;          /* CRC32 of entire file (this field zeroed) */
	uint8_t  arrReserved[12]; /* reserved, zero-filled */
} xanim_header_t;

/* ------------------------------------------------------------------ */
/* TOC (Table of Contents)                                            */
/* ------------------------------------------------------------------ */

#define XANIM_SECTION_META      1
#define XANIM_SECTION_RESOURCE  2
#define XANIM_SECTION_SYMBOL    3
#define XANIM_SECTION_TIMELINE  4
#define XANIM_SECTION_CURVE     5
#define XANIM_SECTION_ACTION    6
#define XANIM_SECTION_STRING    7

typedef struct xanim_toc_entry_t {
	uint32_t iSectionType;  /* XANIM_SECTION_* */
	uint32_t iOffset;       /* byte offset from file start */
	uint32_t iSize;         /* section size in bytes */
	uint32_t iReserved;
} xanim_toc_entry_t;

/* ------------------------------------------------------------------ */
/* Meta Section                                                       */
/* ------------------------------------------------------------------ */

typedef struct xanim_meta_t {
	uint32_t iAuthorIndex;     /* string table index */
	uint32_t iDescriptionIndex;/* string table index */
	uint32_t iCreateDate;      /* unix timestamp */
	uint32_t iModifyDate;      /* unix timestamp */
	uint32_t iBackgroundColor; /* RGBA stage background */
	uint32_t iReserved[3];
} xanim_meta_t;

/* ------------------------------------------------------------------ */
/* Resource Section                                                   */
/* ------------------------------------------------------------------ */

#define XANIM_RES_TEXTURE   1
#define XANIM_RES_FONT      2
#define XANIM_RES_AUDIO     3

#define XANIM_FMT_RGBA8     1
#define XANIM_FMT_A8        2

typedef struct xanim_resource_entry_t {
	uint32_t iId;           /* unique resource id */
	uint32_t iType;         /* XANIM_RES_* */
	uint32_t iFormat;       /* XANIM_FMT_* */
	uint16_t iWidth;
	uint16_t iHeight;
	uint32_t iDataOffset;   /* offset relative to resource section data area */
	uint32_t iDataSize;     /* pixel/audio data size in bytes */
	uint32_t iNameIndex;    /* string table index */
} xanim_resource_entry_t;

/* ------------------------------------------------------------------ */
/* Symbol Section                                                     */
/* ------------------------------------------------------------------ */

#define XANIM_SYMBOL_GRAPHIC     1  /* static vector graphic */
#define XANIM_SYMBOL_SPRITE      2  /* texture sprite */
#define XANIM_SYMBOL_GROUP       3  /* display object group */
#define XANIM_SYMBOL_MOVIE_CLIP  4  /* movie clip with own timeline */

typedef struct xanim_symbol_t {
	uint32_t iId;
	uint32_t iType;          /* XANIM_SYMBOL_* */
	uint32_t iNameIndex;     /* string table index */
	float    fBoundsX;
	float    fBoundsY;
	float    fBoundsW;
	float    fBoundsH;
	uint32_t iChildCount;    /* for GROUP / MOVIE_CLIP */
	uint32_t iChildOffset;   /* byte offset to child array */
	uint32_t iTimelineIndex; /* for MOVIE_CLIP: timeline index, 0xFFFFFFFF = none */
	uint32_t iDataOffset;    /* byte offset to type-specific payload */
	uint32_t iDataSize;      /* payload size */
} xanim_symbol_t;

/* Symbol child reference (for GROUP / MOVIE_CLIP) */
typedef struct xanim_symbol_child_t {
	uint32_t iSymbolId;      /* referenced symbol */
	float    fTx, fTy;       /* translation */
	float    fScaleX, fScaleY;
	float    fRotation;      /* radians */
	float    fOpacity;
	uint32_t iDepth;         /* z-order within group */
} xanim_symbol_child_t;

/* Sprite payload (follows symbol when type == XANIM_SYMBOL_SPRITE) */
typedef struct xanim_sprite_data_t {
	uint32_t iResourceId;    /* texture resource */
	float    fNineX1, fNineY1, fNineX2, fNineY2; /* nine-patch, 0 = disabled */
	uint32_t iFilter;        /* XANIM_FILTER_NEAREST / LINEAR */
} xanim_sprite_data_t;

#define XANIM_FILTER_NEAREST  1
#define XANIM_FILTER_LINEAR   2

/* ------------------------------------------------------------------ */
/* Vector path serialization (GRAPHIC symbol payload)                  */
/* ------------------------------------------------------------------ */

/* Path commands: 1-byte opcode + variable float params */
#define XANIM_PATH_MOVE_TO    1   /* + 2 float (x, y) */
#define XANIM_PATH_LINE_TO    2   /* + 2 float (x, y) */
#define XANIM_PATH_QUAD_TO    3   /* + 4 float (cx, cy, x, y) */
#define XANIM_PATH_CUBIC_TO   4   /* + 6 float (c1x, c1y, c2x, c2y, x, y) */
#define XANIM_PATH_CLOSE      5   /* no params */

/* Paint types (follow path data) */
#define XANIM_PAINT_NONE      0
#define XANIM_PAINT_SOLID     1   /* + uint32 color */
#define XANIM_PAINT_LINEAR    2   /* + xanim_gradient_t */
#define XANIM_PAINT_RADIAL    3   /* + xanim_gradient_t */
#define XANIM_PAINT_TEXTURE   4   /* + uint32 resource_id + 6 float matrix */

typedef struct xanim_gradient_stop_t {
	float    fOffset;        /* 0..1 */
	uint32_t iColor;         /* RGBA */
} xanim_gradient_stop_t;

typedef struct xanim_gradient_t {
	float    fX1, fY1;       /* linear start / radial center */
	float    fX2, fY2;       /* linear end / radial focal */
	float    fRadius;        /* radial radius */
	uint32_t iStopCount;
	/* followed by xanim_gradient_stop_t[iStopCount] */
} xanim_gradient_t;

/* Shape record: one sub-path with fill + stroke */
typedef struct xanim_shape_record_t {
	uint32_t iPathSize;       /* bytes of path command stream */
	uint8_t  iFillType;       /* XANIM_PAINT_* */
	uint8_t  iStrokeType;     /* XANIM_PAINT_* */
	uint8_t  iFillRule;       /* 0=nonzero, 1=evenodd */
	uint8_t  iReserved;
	float    fStrokeWidth;
	uint32_t iLineCap;        /* 0=butt, 1=round, 2=square */
	uint32_t iLineJoin;       /* 0=miter, 1=round, 2=bevel */
	float    fMiterLimit;
	/* followed by: path data [iPathSize bytes] */
	/* followed by: fill paint data (variable) */
	/* followed by: stroke paint data (variable) */
} xanim_shape_record_t;

/* Graphic symbol payload header */
typedef struct xanim_graphic_data_t {
	uint32_t iShapeCount;
	/* followed by xanim_shape_record_t[iShapeCount] + their payloads */
} xanim_graphic_data_t;

/* ------------------------------------------------------------------ */
/* Timeline Section                                                   */
/* ------------------------------------------------------------------ */

#define XANIM_LAYER_NORMAL    0
#define XANIM_LAYER_GUIDE     1
#define XANIM_LAYER_MASK      2
#define XANIM_LAYER_MASKED    3
#define XANIM_LAYER_FOLDER    4

#define XANIM_LAYER_FLAG_VISIBLE   0x0001u
#define XANIM_LAYER_FLAG_LOCKED    0x0002u
#define XANIM_LAYER_FLAG_OUTLINE   0x0004u

typedef struct xanim_layer_t {
	uint32_t iId;
	uint32_t iNameIndex;      /* string table */
	uint32_t iType;           /* XANIM_LAYER_* */
	uint32_t iParentLayer;    /* for MASKED: mask layer id, 0xFFFFFFFF = none */
	uint32_t iKeyframeCount;
	uint32_t iKeyframeOffset; /* byte offset to keyframe array */
	uint32_t iFlags;          /* XANIM_LAYER_FLAG_* */
	uint32_t iColor;          /* layer outline/onion color RGBA */
} xanim_layer_t;

#define XANIM_KF_BLANK     0x0001u
#define XANIM_KF_ANCHOR    0x0002u

typedef struct xanim_keyframe_t {
	uint32_t iFrame;          /* frame number (0-based) */
	uint32_t iElementCount;   /* display objects on this keyframe */
	uint32_t iElementOffset;  /* byte offset to element array */
	uint32_t iLabelIndex;     /* frame label string index, 0xFFFFFFFF = none */
	uint32_t iFlags;          /* XANIM_KF_* */
} xanim_keyframe_t;

#define XANIM_ELEM_FLAG_VISIBLE   0x0001u
#define XANIM_ELEM_FLAG_LOCKED    0x0002u
#define XANIM_ELEM_FLAG_HAS_FILTER 0x0004u

typedef struct xanim_element_t {
	uint32_t iSymbolId;       /* symbol reference */
	uint32_t iInstanceId;     /* unique instance id */
	float    fTx, fTy;        /* translation */
	float    fScaleX, fScaleY;
	float    fRotation;       /* radians */
	float    fSkewX, fSkewY;  /* radians */
	float    fOpacity;        /* 0..1 */
	float    fPivotX, fPivotY;/* transform pivot */
	uint32_t iColorTransform; /* tint color RGBA (alpha = tint strength) */
	uint32_t iTweenId;        /* curve id, 0xFFFFFFFF = none */
	uint32_t iBlendMode;      /* XGE blend mode */
	uint32_t iFlags;          /* XANIM_ELEM_FLAG_* */
	uint32_t iDepth;          /* z-order within keyframe */
} xanim_element_t;

/* ------------------------------------------------------------------ */
/* Animation Curves Section                                           */
/* ------------------------------------------------------------------ */

/* Property channels */
#define XANIM_CHANNEL_X         0
#define XANIM_CHANNEL_Y         1
#define XANIM_CHANNEL_SCALE_X   2
#define XANIM_CHANNEL_SCALE_Y   3
#define XANIM_CHANNEL_ROTATION  4
#define XANIM_CHANNEL_OPACITY   5
#define XANIM_CHANNEL_SKEW_X    6
#define XANIM_CHANNEL_SKEW_Y    7
#define XANIM_CHANNEL_PIVOT_X   8
#define XANIM_CHANNEL_PIVOT_Y   9
#define XANIM_CHANNEL_COLOR_R  10
#define XANIM_CHANNEL_COLOR_G  11
#define XANIM_CHANNEL_COLOR_B  12
#define XANIM_CHANNEL_COLOR_A  13
#define XANIM_CHANNEL_COUNT    14

/* Shape tween vertex channels start at this base */
#define XANIM_CHANNEL_SHAPE_BASE  100

/* Interpolation types */
#define XANIM_INTERP_HOLD       0
#define XANIM_INTERP_LINEAR     1
#define XANIM_INTERP_EASE_IN    2
#define XANIM_INTERP_EASE_OUT   3
#define XANIM_INTERP_EASE_BOTH  4
#define XANIM_INTERP_BEZIER     5
#define XANIM_INTERP_ELASTIC    6
#define XANIM_INTERP_BOUNCE     7
#define XANIM_INTERP_BACK       8

typedef struct xanim_curve_t {
	uint32_t iId;
	uint32_t iElementId;      /* target element instance */
	uint32_t iChannelMask;    /* bitmask of animated channels */
	uint32_t iKeyCount;
	uint32_t iKeyOffset;      /* byte offset to key array */
} xanim_curve_t;

typedef struct xanim_curve_key_t {
	uint32_t iFrame;
	uint32_t iInterpType;     /* XANIM_INTERP_* */
	float    arrValues[XANIM_CHANNEL_COUNT]; /* channel values (compact by mask) */
	float    fEaseIn;         /* 0..1 */
	float    fEaseOut;        /* 0..1 */
	float    arrBezier[4];    /* custom bezier (x1,y1,x2,y2) for INTERP_BEZIER */
} xanim_curve_key_t;

/* ------------------------------------------------------------------ */
/* Action Section                                                     */
/* ------------------------------------------------------------------ */

#define XANIM_ACTION_STOP        1
#define XANIM_ACTION_PLAY        2
#define XANIM_ACTION_GOTO_FRAME  3
#define XANIM_ACTION_GOTO_LABEL  4
#define XANIM_ACTION_LOOP        5
#define XANIM_ACTION_EVENT       6
#define XANIM_ACTION_SOUND       7

typedef struct xanim_action_t {
	uint32_t iFrame;
	uint32_t iLayerId;
	uint32_t iType;          /* XANIM_ACTION_* */
	int32_t  iParam;         /* frame number / loop count */
	uint32_t iNameIndex;     /* event name / label (string table) */
} xanim_action_t;

/* ------------------------------------------------------------------ */
/* String Table                                                       */
/* ------------------------------------------------------------------ */

typedef struct xanim_string_table_t {
	uint32_t iCount;         /* number of strings */
	uint32_t iDataOffset;    /* offset to char data (relative to section) */
	uint32_t iDataSize;      /* total char data bytes */
	/* followed by: uint32_t offsets[iCount] (relative to data start) */
	/* followed by: char data[iDataSize] (null-terminated strings) */
} xanim_string_table_t;

/* ------------------------------------------------------------------ */
/* Runtime Player API                                                 */
/* ------------------------------------------------------------------ */

typedef struct xanim_player_t xanim_player_t;
typedef xanim_player_t* xanim_player;

typedef void (*xanim_event_proc)(xanim_player pPlayer, uint32_t iFrame, const char* sEvent, void* pUser);

/* Lifecycle */
int  xanimPlayerCreate(xanim_player* ppPlayer);
void xanimPlayerDestroy(xanim_player pPlayer);
int  xanimPlayerLoad(xanim_player pPlayer, const char* sPath);
int  xanimPlayerLoadMemory(xanim_player pPlayer, const void* pData, int iSize);
void xanimPlayerUnload(xanim_player pPlayer);

/* Playback control */
int  xanimPlayerPlay(xanim_player pPlayer);
int  xanimPlayerStop(xanim_player pPlayer);
int  xanimPlayerPause(xanim_player pPlayer);
int  xanimPlayerGotoFrame(xanim_player pPlayer, uint32_t iFrame);
int  xanimPlayerGotoLabel(xanim_player pPlayer, const char* sLabel);
void xanimPlayerSetLoop(xanim_player pPlayer, int bLoop);
void xanimPlayerSetSpeed(xanim_player pPlayer, float fSpeed);
int  xanimPlayerIsPlaying(xanim_player pPlayer);
uint32_t xanimPlayerGetFrame(xanim_player pPlayer);
uint32_t xanimPlayerGetFrameCount(xanim_player pPlayer);
float xanimPlayerGetFrameRate(xanim_player pPlayer);

/* Per-frame update (call in main loop) */
int  xanimPlayerUpdate(xanim_player pPlayer, float fDelta);

/* Render (draws into current XGE render context via ShapeEx) */
int  xanimPlayerRender(xanim_player pPlayer, float fX, float fY);
int  xanimPlayerRenderEx(xanim_player pPlayer, const void* pMatrix); /* xge_shape_ex_matrix_t* */

/* Query */
int  xanimPlayerGetSize(xanim_player pPlayer, float* pW, float* pH);
int  xanimPlayerGetSymbolCount(xanim_player pPlayer);
int  xanimPlayerGetInstanceBounds(xanim_player pPlayer, uint32_t iInstanceId, float* pX, float* pY, float* pW, float* pH);
int  xanimPlayerHitTest(xanim_player pPlayer, float fX, float fY, uint32_t* pInstanceId);

/* Instance control (runtime interaction) */
int  xanimPlayerInstanceSetVisible(xanim_player pPlayer, uint32_t iInstanceId, int bVisible);
int  xanimPlayerInstanceSetOpacity(xanim_player pPlayer, uint32_t iInstanceId, float fOpacity);
int  xanimPlayerInstanceSetTransform(xanim_player pPlayer, uint32_t iInstanceId, const void* pMatrix); /* xge_shape_ex_matrix_t* */

/* Event callback */
void xanimPlayerSetEventCallback(xanim_player pPlayer, xanim_event_proc proc, void* pUser);

/* ------------------------------------------------------------------ */
/* Utility: CRC32                                                     */
/* ------------------------------------------------------------------ */

uint32_t xanimCrc32(const void* pData, uint32_t iSize);

#ifdef __cplusplus
}
#endif

#endif /* XANIM_H */
