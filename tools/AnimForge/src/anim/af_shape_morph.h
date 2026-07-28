/*
 * af_shape_morph.h - Shape tween vertex interpolation
 */

#ifndef AF_SHAPE_MORPH_H
#define AF_SHAPE_MORPH_H

#include "../core/af_doc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Shape hint for guided morphing */
typedef struct af_shape_hint_t {
	int   iVertexIndex;   /* vertex index in source path */
	float fOffsetX;       /* target offset X */
	float fOffsetY;       /* target offset Y */
} af_shape_hint_t;

/* Compatibility check */
int afShapeMorphCheckCompatible(const af_path_t* pFrom, const af_path_t* pTo);

/* Resample path to target point count */
int afShapeMorphResample(const af_path_t* pSrc, af_path_t* pDst, int iTargetPoints);

/* Interpolate between two shape records */
int afShapeMorphInterpolate(const af_shape_record_t* pFrom,
                            const af_shape_record_t* pTo,
                            float fT,
                            af_shape_record_t* pOut);

/* Interpolate entire GRAPHIC symbol */
int afShapeMorphInterpolateSymbol(const af_symbol_t* pFrom,
                                  const af_symbol_t* pTo,
                                  float fT,
                                  af_symbol_t* pOut);

/* Shape hints */
int afShapeMorphApplyHints(af_path_t* pPath, const af_shape_hint_t* pHints, int iHintCount);
int afShapeMorphAutoMatch(const af_path_t* pFrom, const af_path_t* pTo,
                          af_shape_hint_t* pHints, int iMaxHints, int* pHintCount);

#ifdef __cplusplus
}
#endif

#endif /* AF_SHAPE_MORPH_H */
