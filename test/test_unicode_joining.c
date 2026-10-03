#include "../src/xge_unicode_joining.h"
#include <stdio.h>
#include <string.h>
int main(int argc,char** argv)
{
    uint32_t cp;unsigned count=0;
    if(argc==2 && !strcmp(argv[1],"--dump")){
        for(cp=0;cp<=0x10ffff;cp++)if(putchar(__xgeJoiningTransparent(cp)?'T':'.')==EOF)return 1;
        return ferror(stdout)?1:0;
    }
    for(cp=0;cp<=0x10ffff;cp++)count+=(unsigned)__xgeJoiningTransparent(cp);
    if(count!=2224 || !__xgeJoiningTransparent(0x64e) || !__xgeJoiningTransparent(0xe0100) ||
        __xgeJoiningTransparent(0x200c) || __xgeJoiningTransparent(0x200d) ||
        __xgeJoiningTransparent(0x110000) || __xgeJoiningTransparent(UINT32_MAX))return 1;
    puts("Actual C joining transparency: 2224 Unicode 17 scalars, ZWJ/ZWNJ are effective neighbors");return 0;
}
