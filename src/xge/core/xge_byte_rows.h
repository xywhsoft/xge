#ifndef XGE_BYTE_ROWS_INTERNAL_H
#define XGE_BYTE_ROWS_INTERNAL_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
/* Required bytes exclude padding after the final row. Nonpositive stride
 * keeps the native tight-row default. Nothing is read or written on failure. */
static inline bool __xgeByteRowsLayout(size_t row,int rows,int stride,size_t capacity,size_t* required,int* normalized)
{
    if(!row || row>(size_t)INT_MAX || rows<=0)return false;
    if(stride<=0)stride=(int)row;
    if((size_t)stride<row || (size_t)(rows-1)>(SIZE_MAX-row)/(size_t)stride)return false;
    size_t bytes=(size_t)(rows-1)*(size_t)stride+row;
    if(bytes>capacity)return false;
    if(required)*required=bytes;
    if(normalized)*normalized=stride;
    return true;
}
#endif
