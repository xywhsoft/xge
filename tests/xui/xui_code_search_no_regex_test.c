#include "xui.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void)
{
 int count=37;char error[64]={0};
 assert(xuiCodeSearchMarkAllRegex(NULL,NULL,"foo",0,0,&count,error,sizeof(error))==XUI_ERROR_UNSUPPORTED);
 assert(count==0&&!strcmp(error,"XRT regex support is disabled"));
 puts("MarkAll disabled-regex source branch: explicit unsupported and zero count passed");return 0;
}
