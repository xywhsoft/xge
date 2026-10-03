/* Compare against the actual pinned HB joining table and category fallback,
 * independently of the Unicode-data generator and C buffer integration. */
#include "../lib/harfbuzz/src/hb.hh"
enum { JOINING_TYPE_U=0, JOINING_TYPE_L=1, JOINING_TYPE_R=2, JOINING_TYPE_D=3,
    JOINING_TYPE_C=3, JOINING_GROUP_ALAPH=4, JOINING_GROUP_DALATH_RISH=5,
    JOINING_TYPE_T=6, JOINING_TYPE_X=7 };
#include "../lib/harfbuzz/src/hb-ot-shaper-arabic-table.hh"
#include "../src/xge_unicode_joining.h"
#include <stdio.h>
int main()
{
    for(uint32_t cp=0;cp<=0x10ffff;cp++){
        unsigned type=joining_type(cp);
        if(type==JOINING_TYPE_X){
            auto category=hb_unicode_general_category(hb_unicode_funcs_get_default(),cp);
            type=(category==HB_UNICODE_GENERAL_CATEGORY_NON_SPACING_MARK ||
                category==HB_UNICODE_GENERAL_CATEGORY_ENCLOSING_MARK ||
                category==HB_UNICODE_GENERAL_CATEGORY_FORMAT)?JOINING_TYPE_T:JOINING_TYPE_U;
        }
        if(__xgeJoiningTransparent(cp)!=(type==JOINING_TYPE_T)){
            fprintf(stderr,"Joining transparency differs from pinned HB at U+%04X\n",cp);return 1;
        }
    }
    puts("All 1114112 Unicode joining queries agree with the pinned HB table and real general-category fallback");return 0;
}
