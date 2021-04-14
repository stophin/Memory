//memprotect
//

#ifndef MEMPROTECT_H
#define MMEPROTECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

//#define P_OVERFLOW	//定义:上溢出  未定义:下溢出

void *pmalloc(int size);
void pfree(void * ptr);

#ifdef __cplusplus
}
#endif

#endif /* MEMPROTECT_H */