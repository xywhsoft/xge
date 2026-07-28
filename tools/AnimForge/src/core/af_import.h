/*
 * af_import.h - SVG / PNG / WAV import
 */

#ifndef AF_IMPORT_H
#define AF_IMPORT_H

#include "../core/af_doc.h"

#ifdef __cplusplus
extern "C" {
#endif

int afImportPNG(af_doc pDoc, const char* sPath, uint32_t* pSymbolId);
int afImportSVG(af_doc pDoc, const char* sPath, uint32_t* pSymbolId);
int afImportWAV(af_doc pDoc, const char* sPath, uint32_t* pResourceId);
int afImportFile(af_doc pDoc, const char* sPath, uint32_t* pId);

#ifdef __cplusplus
}
#endif

#endif /* AF_IMPORT_H */
