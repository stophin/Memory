/* ***** BEGIN LICENSE BLOCK *****
* Source last modified: $Id: tmalloc.h,v 1.0 2019/4/1 19:51:49 $
*
* Portions Copyright (c) 2019-20295 lszhang. All Rights Reserved.
*
* This file, and the files included with this file, is distributed
* and made available on an 'AS IS' basis, WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESS OR IMPLIED, AND REALNETWORKS HEREBY DISCLAIMS
* ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET
* ENJOYMENT OR NON-INFRINGEMENT.
*
* Version: V1.1
* Contributor(s):lszhang
* 1.增加宏UNDEROVERFLOWCHECK支持内存溢出检测。vc已经支持内存溢出检测，
*   故WIN32不再检测。只做8字节内检测。
* 2.增加支持字节对齐开空间函数TALIGNED_MALLOC和释放函数TALIGNED_FREE，
*   例如1024字节对齐。
* 3.如果打印有"TERR: ",证明memory管理出错，需要查找原因。
* Version: V1.0
* Contributor(s):lszhang
* 1.支持内存空间泄漏检测。
* ***** END LICENSE BLOCK ***** */
/*
说明如下：
1、MAXMLLOCNUM：开空间次数最大值，可以自行修改，如果开空间次数超过MAXMLLOCNUM，
则不再进行保存，会每次开空间打印信息，提示是否频繁开空间。
尽量不要在每帧或者循环体里面开空间释放空间。
2、下面的例子是调用例子。需要在代码开始处调用初始化函数TINIT();代码结束处调用退出函数TDEINIT();
3、把自己代码里面的开空间函数进行如下替换即可：
malloc 替换为TMALLOC
free   替换为TFREE
realloc替换为TREALLOC
calloc 替换为TCALLOC
4、如果平台的开空间和释放空间函数不是malloc free realloc calloc。
只需要MALLOC，FREE REALLOC CALLOC重新转定义到平台的空间管理函数即可
*/

/* demo dunction */
/* out printf
TERR: TMALLOC must call TFREE:func:testbuffer,line:112
TERR: TALIGNED_MALLOC must call TALIGNED_FREE:func:testbuffer,line:114
TERR: calloc err:func:testbuffer,line:117,size:1024,must call TALIGNED_FREE:func
:testbuffer,line:116
TSUCCESS: no flow all success!!!!
TERR: no free num:2，malloc total num:7
TERR: func:testbuffer, line:117, p:0X00660530, size:1024, num:5, aligned:0
TERR: func:testbuffer, line:118, p:0X006611C0, size:2048, num:6, aligned:0
TERR: no free total size:3072
请按任意键继续. . .
*/
/*
void test(void)
{
	int num;
	int *ptr0 = NULL;
	int *ptr1 = NULL;
	int *ptr2 = NULL;

	//init
	TINIT();

	 you code start
	//测试代码开始
	ptr0 = TMALLOC(1024);
	ptr1 = TCALLOC(2, 1024);
	ptr0 = TREALLOC(ptr0, 2048);
	TALIGNED_FREE(ptr0);
	ptr0 = TALIGNED_MALLOC(1024, 2048);
	TFREE(ptr0);
	ptr0 = TALIGNED_MALLOC(1024, 512);
	ptr0 = TREALLOC(ptr0, 1024);
	ptr2 = TREALLOC(NULL, 2048);
	TFREE(ptr1);

	// 测试代码结束
	 you code end

    // deinit
	TDEINIT();
}
*/
#ifndef TMEMORY_H
#define TMEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define TDEBUG

#ifdef TDEBUG

//#ifndef WIN32
#define UNDEROVERFLOWCHECK
//#endif

#define MAXMLLOCNUM 100000

#define CALLOC calloc
#define REALLOC realloc
#define MALLOC malloc
#define FREE free
void tinit(void);
void tdeinit(void);
void *tcalloc(char *func, int line, size_t Count, size_t Size);
void *trealloc(char *func, int line, void *Memory, size_t NewSize);
void *tmalloc(char *func, int line, int size);
void tfree(void *ptr);
void *taligned_malloc(char *func, int line, int size, int alignment);
void taligned_free(void *palignedmem);

/*
 * call below functions
 */
#define TINIT tinit
#define TDEINIT tdeinit
#define TALIGNED_MALLOC(size,alignment) taligned_malloc(__FUNCTION__, __LINE__, size,alignment)
#define TALIGNED_FREE(ptr) taligned_free(ptr)
#define TCALLOC(Count,Size) tcalloc(__FUNCTION__, __LINE__, Count,Size)
#define TREALLOC(Memory,NewSize) trealloc(__FUNCTION__, __LINE__, Memory, NewSize)
#define TMALLOC(len) tmalloc(__FUNCTION__, __LINE__, len)
#define TFREE(ptr) tfree(ptr)
#else
#define TINIT
#define TDEINIT
#define TCALLOC calloc
#define TREALLOC realloc
#define TMALLOC malloc
#define TFREE free
#endif

#ifdef __cplusplus
}
#endif

#endif /* TMEMORY_H */
