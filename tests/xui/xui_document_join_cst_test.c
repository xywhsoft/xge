#include "xui_document_partial_entity_cases.h"
#include "xui_document_partial_cross_cases.h"
#include "xui_document_nonempty_seam_cases.h"
#include "xui_document_join_heading_cases.h"
#include "xui_document_join_source_cases.h"
int main(void) { join_goldens(); join_faults(); TABLE_CHECK(!join_continuations()); heading_join_goldens(); heading_join_faults(); heading_edge_goldens(); heading_replace_goldens(); TABLE_CHECK(!nonempty_format_goldens()); TABLE_CHECK(!nonempty_public_faults()); TABLE_CHECK(!nonempty_continuations()); TABLE_CHECK(!partial_strict_nonempty()); TABLE_CHECK(!partial_strict_empty()); TABLE_CHECK(!partial_cross_goldens()); TABLE_CHECK(!partial_public_faults()); TABLE_CHECK(!entity_strict()); TABLE_CHECK(!entity_goldens()); TABLE_CHECK(!entity_public_faults()); return 0; }
