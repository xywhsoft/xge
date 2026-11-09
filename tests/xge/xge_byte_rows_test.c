#include "src/xge/core/xge_byte_rows.h"
#include <assert.h>
#include <stdio.h>
int main(void)
{
 for(size_t row=1;row<=8;++row)for(int rows=1;rows<=8;++rows)for(int stride=-2;stride<=16;++stride)for(size_t capacity=0;capacity<=128;++capacity){
  int actual=stride<=0 ? (int)row:stride;size_t bytes=123456;int normalized=654321;
  bool valid=(size_t)actual>=row && (uint64_t)(rows-1)*(uint64_t)actual+row<=capacity;
  assert(__xgeByteRowsLayout(row,rows,stride,capacity,&bytes,&normalized)==valid);
  if(valid){assert(bytes==(size_t)(rows-1)*(size_t)actual+row && normalized==actual);}
  else assert(bytes==123456 && normalized==654321);
 }
 size_t bytes=0;int stride=0;
 assert(!__xgeByteRowsLayout(0,1,0,SIZE_MAX,NULL,NULL));
 assert(!__xgeByteRowsLayout((size_t)INT_MAX+1u,1,0,SIZE_MAX,NULL,NULL));
 assert(!__xgeByteRowsLayout(1,INT_MIN,0,SIZE_MAX,NULL,NULL));
 uint64_t large=(uint64_t)INT_MAX*(uint64_t)INT_MAX;
 assert(__xgeByteRowsLayout(INT_MAX,INT_MAX,INT_MAX,SIZE_MAX,&bytes,&stride)==(large<=SIZE_MAX));
 if(large<=SIZE_MAX)assert(bytes==large && stride==INT_MAX);
 assert(__xgeByteRowsLayout(4,2,8,12,&bytes,&stride) && bytes==12 && stride==8);
 assert(!__xgeByteRowsLayout(4,2,8,11,NULL,NULL));
 assert(__xgeByteRowsLayout(4,2,INT_MIN,8,&bytes,&stride) && bytes==8 && stride==4);
 puts("byte row arithmetic: exhaustive small ranges, unchanged failure outputs and address-width boundaries passed");return 0;
}
