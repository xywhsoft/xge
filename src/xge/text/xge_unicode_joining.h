/* Unicode joining transparency is different from general mark/category or
 * default-ignorable status: ZWJ and ZWNJ must remain effective neighbors. */
#ifndef XGE_UNICODE_JOINING_H
#define XGE_UNICODE_JOINING_H
#include <stddef.h>
#include <stdint.h>
#include "xge_unicode_joining_data.inc"
static int __xgeJoiningTransparent(uint32_t cp)
{
    size_t low=0,high=sizeof(__xgeJoiningTransparentRanges)/sizeof(*__xgeJoiningTransparentRanges);
    if(cp<0xadu)return 0;
    while(low<high){
        size_t mid=low+(high-low)/2;
        if(cp<__xgeJoiningTransparentRanges[mid].first)high=mid;
        else if(cp>__xgeJoiningTransparentRanges[mid].last)low=mid+1;
        else return 1;
    }
    return 0;
}
#endif
