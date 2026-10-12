/*
 * af_doc.h - AnimForge document model
 *
 * In-memory representation of an animation document.
 * This is the editor's working data, distinct from the on-disk
 * xanim format. The af_file module serializes/deserializes between
 * this model and the .xanim binary format.
 */

#ifndef AF_DOC_H
#define AF_DOC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/* Limits                                                             */
/* ------------------------------------------------------------------ */

#define AF_DOC_MAX_LAYERS       64
#define AF_DOC_MAX_KEYFRAMES    1024
#define AF_DOC_MAX_ELEMENTS     1024
#define AF_DOC_MAX_SYMBOLS      128
#define AF_DOC_MAX_RESOURCES    64
#define AF_DOC_MAX_CURVES       512
#define AF_DOC_MAX_ACTIONS      256
#define AF_DOC_MAX_NAME         128
#define AF_DOC_MAX_PATH_CMDS    512
#define AF_DOC_MAX_PATH_PTS     1024
#define AF_DOC_MAX_SHAPE_RECORDS 8
#define AF_DOC_MAX_GRADIENT_STOPS 16
#define AF_DOC_MAX_CHILDREN     16

/* ------------------------------------------------------------------ */
/* Forward declarations                                               */
/* ------------------------------------------------------------------ */

typedef struct af_doc_t af_doc_t;
typedef af_doc_t* af_doc;

/* ------------------------------------------------------------------ */
/* Vector path (editor representation)                                 */
/* ------------------------------------------------------------------ */

#define AF_PATH_CMD_MOVE_TO   1
#define AF_PATH_CMD_LINE_TO   2
#define AF_PATH_CMD_QUAD_TO   3
#define AF_PATH_CMD_CUBIC_TO  4
#define AF_PATH_CMD_CLOSE     5

typedef struct af_path_t {
	uint8_t  arrCommands[AF_DOC_MAX_PATH_CMDS];
	int      iCommandCount;
	float    arrPoints[AF_DOC_MAX_PATH_PTS * 2]; /* x,y pairs */
	int      iPointCount;
} af_path_t;

/* ------------------------------------------------------------------ */
/* Paint (fill / stroke)                                              */
/* ------------------------------------------------------------------ */

#define AF_PAINT_NONE      0
#define AF_PAINT_SOLID     1
#define AF_PAINT_LINEAR    2
#define AF_PAINT_RADIAL    3
#define AF_PAINT_TEXTURE   4

typedef struct af_gradient_stop_t {
	float    fOffset;
	uint32_t iColor;
} af_gradient_stop_t;

typedef struct af_paint_t {
	int      iType;          /* AF_PAINT_* */
	uint32_t iColor;         /* solid color RGBA */
	float    fX1, fY1;       /* gradient start / center */
	float    fX2, fY2;       /* gradient end / focal */
	float    fRadius;        /* radial radius */
	int      iStopCount;
	af_gradient_stop_t arrStops[AF_DOC_MAX_GRADIENT_STOPS];
	uint32_t iTextureId;     /* for PAINT_TEXTURE */
} af_paint_t;

/* ------------------------------------------------------------------ */
/* Shape record (one sub-path with fill + stroke)                     */
/* ------------------------------------------------------------------ */

typedef struct af_shape_record_t {
	af_path_t tPath;
	af_paint_t tFill;
	af_paint_t tStroke;
	float    fStrokeWidth;
	int      iFillRule;      /* 0=nonzero, 1=evenodd */
	int      iLineCap;       /* 0=butt, 1=round, 2=square */
	int      iLineJoin;      /* 0=miter, 1=round, 2=bevel */
	float    fMiterLimit;
} af_shape_record_t;

/* ------------------------------------------------------------------ */
/* Symbol                                                             */
/* ------------------------------------------------------------------ */

#define AF_SYMBOL_GRAPHIC     1
#define AF_SYMBOL_SPRITE      2
#define AF_SYMBOL_GROUP       3
#define AF_SYMBOL_MOVIE_CLIP  4

typedef struct af_symbol_child_t {
	uint32_t iSymbolId;
	float    fTx, fTy;
	float    fScaleX, fScaleY;
	float    fRotation;
	float    fOpacity;
	uint32_t iDepth;
} af_symbol_child_t;

typedef struct af_symbol_t {
	uint32_t iId;
	int      iType;          /* AF_SYMBOL_* */
	char     sName[AF_DOC_MAX_NAME];
	float    fBoundsX, fBoundsY, fBoundsW, fBoundsH;

	/* GRAPHIC payload */
	int      iShapeCount;
	af_shape_record_t arrShapes[AF_DOC_MAX_SHAPE_RECORDS];

	/* SPRITE payload */
	uint32_t iResourceId;
	float    fNineX1, fNineY1, fNineX2, fNineY2;

	/* GROUP / MOVIE_CLIP children */
	int      iChildCount;
	af_symbol_child_t arrChildren[AF_DOC_MAX_CHILDREN];

	/* MOVIE_CLIP: index into doc timeline array (0 = main, >0 = nested) */
	int      iTimelineIndex;
} af_symbol_t;

/* ------------------------------------------------------------------ */
/* Resource                                                           */
/* ------------------------------------------------------------------ */

#define AF_RES_TEXTURE   1
#define AF_RES_FONT      2
#define AF_RES_AUDIO     3

typedef struct af_resource_t {
	uint32_t iId;
	int      iType;          /* AF_RES_* */
	int      iFormat;        /* pixel format */
	int      iWidth;
	int      iHeight;
	char     sName[AF_DOC_MAX_NAME];
	void*    pData;          /* owned pixel/audio data */
	int      iDataSize;
} af_resource_t;

/* ------------------------------------------------------------------ */
/* Timeline elements                                                  */
/* ------------------------------------------------------------------ */

#define AF_ELEM_FLAG_VISIBLE    0x0001u
#define AF_ELEM_FLAG_LOCKED     0x0002u
#define AF_ELEM_FLAG_HAS_FILTER 0x0004u

typedef struct af_element_t {
	uint32_t iSymbolId;
	uint32_t iInstanceId;
	float    fTx, fTy;
	float    fScaleX, fScaleY;
	float    fRotation;
	float    fSkewX, fSkewY;
	float    fOpacity;
	float    fPivotX, fPivotY;
	uint32_t iColorTransform;
	uint32_t iTweenId;       /* curve id, 0xFFFFFFFF = none */
	uint32_t iBlendMode;
	uint32_t iFlags;
	uint32_t iDepth;
} af_element_t;

/* ------------------------------------------------------------------ */
/* Keyframe                                                           */
/* ------------------------------------------------------------------ */

#define AF_KF_BLANK    0x0001u
#define AF_KF_ANCHOR   0x0002u

typedef struct af_keyframe_t {
	uint32_t iFrame;
	int      iElementCount;
	af_element_t arrElements[16]; /* elements on this keyframe */
	char     sLabel[AF_DOC_MAX_NAME];
	uint32_t iFlags;
} af_keyframe_t;

/* ------------------------------------------------------------------ */
/* Layer                                                              */
/* ------------------------------------------------------------------ */

#define AF_LAYER_NORMAL    0
#define AF_LAYER_GUIDE     1
#define AF_LAYER_MASK      2
#define AF_LAYER_MASKED    3
#define AF_LAYER_FOLDER    4

#define AF_LAYER_FLAG_VISIBLE  0x0001u
#define AF_LAYER_FLAG_LOCKED   0x0002u
#define AF_LAYER_FLAG_OUTLINE  0x0004u

typedef struct af_layer_t {
	uint32_t iId;
	char     sName[AF_DOC_MAX_NAME];
	int      iType;           /* AF_LAYER_* */
	uint32_t iParentLayer;    /* for MASKED */
	int      iKeyframeCount;
	af_keyframe_t arrKeyframes[AF_DOC_MAX_KEYFRAMES / AF_DOC_MAX_LAYERS];
	uint32_t iFlags;
	uint32_t iColor;          /* layer color for onion/outline */
} af_layer_t;

/* ------------------------------------------------------------------ */
/* Animation curve                                                    */
/* ------------------------------------------------------------------ */

#define AF_CHANNEL_COUNT  14

typedef struct af_curve_key_t {
	uint32_t iFrame;
	uint32_t iInterpType;
	float    arrValues[AF_CHANNEL_COUNT];
	float    fEaseIn;
	float    fEaseOut;
	float    arrBezier[4];
} af_curve_key_t;

#define AF_CURVE_MAX_KEYS  64

typedef struct af_curve_t {
	uint32_t iId;
	uint32_t iElementId;
	uint32_t iChannelMask;
	int      iKeyCount;
	af_curve_key_t arrKeys[AF_CURVE_MAX_KEYS];
} af_curve_t;

/* ------------------------------------------------------------------ */
/* Action                                                             */
/* ------------------------------------------------------------------ */

typedef struct af_action_t {
	uint32_t iFrame;
	uint32_t iLayerId;
	uint32_t iType;
	int32_t  iParam;
	char     sName[AF_DOC_MAX_NAME];
} af_action_t;

/* ------------------------------------------------------------------ */
/* Timeline (main or nested in MovieClip)                             */
/* ------------------------------------------------------------------ */

typedef struct af_timeline_t {
	int      iLayerCount;
	af_layer_t arrLayers[AF_DOC_MAX_LAYERS];
} af_timeline_t;

/* ------------------------------------------------------------------ */
/* Document                                                           */
/* ------------------------------------------------------------------ */

struct af_doc_t {
	/* Document properties */
	char     sName[AF_DOC_MAX_NAME];
	char     sFilePath[512];
	float    fFrameRate;
	uint32_t iFrameCount;
	float    fStageWidth;
	float    fStageHeight;
	uint32_t iBackgroundColor;
	uint32_t iFlags;          /* XANIM_FLAG_* */
	int      bModified;

	/* Resources */
	int      iResourceCount;
	af_resource_t arrResources[AF_DOC_MAX_RESOURCES];

	/* Symbols */
	int      iSymbolCount;
	af_symbol_t arrSymbols[AF_DOC_MAX_SYMBOLS];

	/* Main timeline */
	af_timeline_t tTimeline;

	/* Curves */
	int      iCurveCount;
	af_curve_t arrCurves[AF_DOC_MAX_CURVES];

	/* Actions */
	int      iActionCount;
	af_action_t arrActions[AF_DOC_MAX_ACTIONS];

	/* ID counters */
	uint32_t iNextSymbolId;
	uint32_t iNextInstanceId;
	uint32_t iNextCurveId;
	uint32_t iNextResourceId;
	uint32_t iNextLayerId;
};

/* ------------------------------------------------------------------ */
/* Document API                                                       */
/* ------------------------------------------------------------------ */

int  afDocInit(af_doc pDoc);
void afDocClear(af_doc pDoc);
int  afDocSetStage(af_doc pDoc, float fWidth, float fHeight, float fFrameRate, uint32_t iFrameCount);

/* Symbol management */
int  afDocAddSymbol(af_doc pDoc, int iType, const char* sName, uint32_t* pId);
int  afDocRemoveSymbol(af_doc pDoc, uint32_t iId);
af_symbol_t* afDocFindSymbol(af_doc pDoc, uint32_t iId);

/* Resource management */
int  afDocAddResource(af_doc pDoc, int iType, const char* sName, void* pData, int iDataSize, int iWidth, int iHeight, uint32_t* pId);
int  afDocRemoveResource(af_doc pDoc, uint32_t iId);
af_resource_t* afDocFindResource(af_doc pDoc, uint32_t iId);

/* Layer management */
int  afDocAddLayer(af_doc pDoc, const char* sName, int iType, uint32_t* pId);
int  afDocRemoveLayer(af_doc pDoc, uint32_t iId);
af_layer_t* afDocFindLayer(af_doc pDoc, uint32_t iId);
int  afDocMoveLayer(af_doc pDoc, uint32_t iId, int iDirection);

/* Keyframe management */
int  afDocSetKeyframe(af_doc pDoc, uint32_t iLayerId, uint32_t iFrame, uint32_t iFlags);
int  afDocClearKeyframe(af_doc pDoc, uint32_t iLayerId, uint32_t iFrame);
af_keyframe_t* afDocGetKeyframe(af_doc pDoc, uint32_t iLayerId, uint32_t iFrame);

/* Element management */
int  afDocAddElement(af_doc pDoc, uint32_t iLayerId, uint32_t iFrame, uint32_t iSymbolId, uint32_t* pInstanceId);
int  afDocRemoveElement(af_doc pDoc, uint32_t iLayerId, uint32_t iFrame, uint32_t iInstanceId);
af_element_t* afDocFindElement(af_doc pDoc, uint32_t iInstanceId);

/* Curve management */
int  afDocAddCurve(af_doc pDoc, uint32_t iElementId, uint32_t iChannelMask, uint32_t* pId);
int  afDocRemoveCurve(af_doc pDoc, uint32_t iId);
af_curve_t* afDocFindCurve(af_doc pDoc, uint32_t iId);

/* Action management */
int  afDocAddAction(af_doc pDoc, uint32_t iFrame, uint32_t iLayerId, uint32_t iType, int32_t iParam, const char* sName);
int  afDocRemoveAction(af_doc pDoc, int iIndex);

#ifdef __cplusplus
}
#endif

#endif /* AF_DOC_H */
