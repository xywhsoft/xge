#ifndef XUI_TEXT_DISPLAY_H
#define XUI_TEXT_DISPLAY_H
#include "../xui.h"
/* Display deletion policy shared by transient text layout and retained
 * Document projections. ZWJ/ZWNJ remain available to the shaping backend. */
static inline int __xuiTextCopyDisplay(const char* text,int bytes,char* output)
{
    int at=0,count=0;
    if(!text || bytes<0)return XUI_ERROR_INVALID_ARGUMENT;
    while(at<bytes){
        int length=0;const unsigned char* p=(const unsigned char*)text+at;
        if(bytes-at>=2 && ((p[0]==0xc2 && p[1]==0xad) || (p[0]==0xd8 && p[1]==0x9c)))length=2;
        else if(bytes-at>=3 &&
            ((p[0]==0xe2 && p[1]==0x80 && (p[2]==0x8b || p[2]==0x8e || p[2]==0x8f || (p[2]>=0xaa && p[2]<=0xae))) ||
             (p[0]==0xe2 && p[1]==0x81 && (p[2]==0xa0 || (p[2]>=0xa6 && p[2]<=0xa9))) ||
             (p[0]==0xef && p[1]==0xbb && p[2]==0xbf)))length=3;
        if(length)at+=length;
        else{if(output)output[count]=text[at];count++;at++;}
    }
    return count;
}
#endif
