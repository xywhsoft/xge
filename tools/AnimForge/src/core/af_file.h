/*
 * af_file.h - AnimForge .xanim file serialization
 *
 * Reads/writes the af_doc_t in-memory model to/from the .xanim
 * binary format defined in xanim.h.
 */

#ifndef AF_FILE_H
#define AF_FILE_H

#include "af_doc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Save document to .xanim file. Returns 0 on success, -1 on error. */
int afFileSave(af_doc pDoc, const char* sPath);

/* Load document from .xanim file. Returns 0 on success, -1 on error. */
int afFileLoad(af_doc pDoc, const char* sPath);

#ifdef __cplusplus
}
#endif

#endif /* AF_FILE_H */
