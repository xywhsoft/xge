/*
 * af_export.h - PNG sequence / GIF / Sprite sheet export
 */

#ifndef AF_EXPORT_H
#define AF_EXPORT_H

#include "../core/af_doc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Export types */
#define AF_EXPORT_TYPE_XANIM         0
#define AF_EXPORT_TYPE_PNG_SEQ       1
#define AF_EXPORT_TYPE_GIF           2
#define AF_EXPORT_TYPE_SPRITE_SHEET  3

int afExportPNGSequence(af_doc pDoc, const char* sDir, int iStartFrame,
                        int iEndFrame, float fScale);
int afExportGIF(af_doc pDoc, const char* sPath, int iStartFrame,
                int iEndFrame, float fScale, int iDelay);
int afExportSpriteSheet(af_doc pDoc, const char* sPath, int iStartFrame,
                        int iEndFrame, int iCols, float fScale);
int afExport(af_doc pDoc, int iType, const char* sPath,
             int iStartFrame, int iEndFrame, float fScale);

#ifdef __cplusplus
}
#endif

#endif /* AF_EXPORT_H */
