/*
 * af_symbol.h - Symbol definition and instance management
 */

#ifndef AF_SYMBOL_H
#define AF_SYMBOL_H

#include "../core/af_doc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Symbol creation */
int afSymbolCreateGraphic(af_doc pDoc, const char* sName, uint32_t* pId);
int afSymbolCreateSprite(af_doc pDoc, const char* sName, uint32_t iResourceId, uint32_t* pId);
int afSymbolCreateGroup(af_doc pDoc, const char* sName, uint32_t* pId);
int afSymbolCreateMovieClip(af_doc pDoc, const char* sName, uint32_t* pId);

/* Group child management */
int afSymbolAddChild(af_doc pDoc, uint32_t iGroupId, uint32_t iChildSymbolId,
                     float fTx, float fTy, uint32_t iDepth);
int afSymbolRemoveChild(af_doc pDoc, uint32_t iGroupId, int iChildIndex);
int afSymbolMoveChild(af_doc pDoc, uint32_t iGroupId, int iFrom, int iTo);

/* Shape record management (GRAPHIC symbols) */
int afSymbolAddShape(af_doc pDoc, uint32_t iSymbolId, const af_shape_record_t* pShape, int* pIndex);
int afSymbolRemoveShape(af_doc pDoc, uint32_t iSymbolId, int iShapeIndex);

/* Bounds */
int afSymbolRecalcBounds(af_doc pDoc, uint32_t iSymbolId);

/* Instance placement */
int afSymbolInstantiate(af_doc pDoc, uint32_t iSymbolId, uint32_t iLayerId,
                        uint32_t iFrame, float fX, float fY, uint32_t* pInstanceId);

/* Duplication / conversion */
int afSymbolDuplicate(af_doc pDoc, uint32_t iSrcId, const char* sNewName, uint32_t* pNewId);
int afSymbolConvertToSymbol(af_doc pDoc, uint32_t iLayerId, uint32_t iFrame,
                            int iType, const char* sName, uint32_t* pSymbolId);

/* Query */
int afSymbolGetUsageCount(af_doc pDoc, uint32_t iSymbolId);
const char* afSymbolTypeName(int iType);

#ifdef __cplusplus
}
#endif

#endif /* AF_SYMBOL_H */
