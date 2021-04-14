/* ***** BEGIN LICENSE BLOCK *****
* Source last modified: $Id: tmalloc.c,v 1.0 2019/4/1 19:51:49 $
*
* Portions Copyright (c) 2019-2029 lszhang. All Rights Reserved.
*
* This file, and the files included with this file, is distributed
* and made available on an 'AS IS' basis, WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESS OR IMPLIED, AND REALNETWORKS HEREBY DISCLAIMS
* ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET
* ENJOYMENT OR NON-INFRINGEMENT.
*
* Version: V1.0
* Contributor(s):lszhang
*
* ***** END LICENSE BLOCK ***** */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tmemory.h"

#ifdef TDEBUG

#define STRINGLEN 127
#define OVERFLOW 1
#define UNDERFLOW 2
#define CHECKDATALEN 8

#ifdef UNDEROVERFLOWCHECK

#define EXTRADATALEN (2 * CHECKDATALEN)
#define FLOWSIZE CHECKDATALEN
#else
#define EXTRADATALEN 0
#define FLOWSIZE 0
#endif

typedef struct vars {
	char func[STRINGLEN + 1];
	int line;
	void *ptr;
	int size;
	int num;
	int flow;
	int aligned;
} vars;

typedef struct mem_st {
	vars var[MAXMLLOCNUM];
	int lost;
	int underflow;
	int overflow;
	int total;
} mem_st;

static mem_st tmemory;
static const unsigned char tmemory_data[CHECKDATALEN] = {
	0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef
};
void tinit(void)
{
	memset(&tmemory, 0, sizeof(mem_st));
}

void *tmemorydata(void *ptr, size_t Size)
{
	unsigned char *pchar = ptr;
	if (!ptr)
		return ptr;
	if (FLOWSIZE != 0) {
		memcpy(pchar, tmemory_data, FLOWSIZE);
		memcpy(pchar + Size + FLOWSIZE, tmemory_data, FLOWSIZE);
	}
	return (void *)(pchar + FLOWSIZE);
}
void *tmemoryflow(void *ptr, int *pnum)
{
	int i = 0;
	int j = 0;
	unsigned char *pchar = ptr;
	int num = 0;
	int flow = 0;
	int Size = 0;
	if (!ptr)
		return NULL;

	pchar -= FLOWSIZE;
	for (i = 0; i < FLOWSIZE; i++) {
		if (pchar[i] != tmemory_data[i]) {
			flow = UNDERFLOW;
			printf("TERR: CRT detected that the application wrote to memory before start of heap buffer.\n");
			tmemory.underflow++;
			break;
		}
	}

	num = tmemory.total > MAXMLLOCNUM ? MAXMLLOCNUM : tmemory.total;
	for (i = 0; i < num; i++) {
		if (tmemory.var[i].ptr == ptr) {
			Size = tmemory.var[i].size;
			for (j = 0; j < FLOWSIZE; j++) {
				if (pchar[Size + FLOWSIZE + j] != tmemory_data[j]) {
					flow = OVERFLOW;
					printf("TERR: CRT detected that the application wrote to memory after end of heap buffer.\n");
					tmemory.overflow++;
					break;
				}
			}
			if (flow) {
				printf("TERR: err-flow func:%s, line:%d, p:%#p, size:%d, num:%d,flow:%d\n",
					tmemory.var[i].func,
					tmemory.var[i].line,
					tmemory.var[i].ptr,
					tmemory.var[i].size,
					tmemory.var[i].num,
					flow);
			}
			*pnum = i;
			tmemory.var[i].ptr = 0;
			tmemory.var[i].flow = flow;
			tmemory.lost--;
			if (tmemory.var[i].aligned)
				return NULL;
			break;
		}
	}
	if (flow && (i == num)) {
		printf("TERR: err-flow  p:%#p, size:%d\n", ptr, Size);
	}

	return (void *)pchar;
}

void tdeinit(void)
{
	int i = 0;
	int total = 0;

	if (tmemory.total){
		if (FLOWSIZE != 0) {
			if (tmemory.overflow || tmemory.underflow) {
				printf("TERR: err-flow:overflow num:%d. underflow num:%d\n", tmemory.overflow,
					tmemory.underflow);
			} else {
				printf("TSUCCESS: no flow all success!!!!\n");
			}
		}

		if (tmemory.lost == 0) {
			printf("TSUCCESS:free all success!!!!,malloc total num:%d\n", tmemory.total);
		} else {
			int lostnum = 0;
			int num = tmemory.total > MAXMLLOCNUM ? MAXMLLOCNUM : tmemory.total;
			printf("TERR: no free num:%d，malloc total num:%d\n", tmemory.lost, tmemory.total);
			for (i = 0; i < num; i++) {
				if (tmemory.var[i].ptr) {
					printf("TERR: func:%s, line:%d, p:%#p, size:%d, num:%d, aligned:%d\n",
						tmemory.var[i].func,
						tmemory.var[i].line,
						tmemory.var[i].ptr,
						tmemory.var[i].size,
						tmemory.var[i].num,
						tmemory.var[i].aligned);
					lostnum++;
					total += tmemory.var[i].size;
				}
			}
			if (lostnum != tmemory.lost) {
				printf("TERR: printf num :%d,no printf num:%d\n", lostnum, tmemory.lost - lostnum);
			}
			printf("TERR: no free total size:%d\n", total);
		}
	}	
}

void *tcalloc(char *func, int line, size_t Count, size_t Size)
{
	void *ptr = CALLOC(Count, Size + EXTRADATALEN);
	if (!ptr) {
		printf("TERR: calloc err:func:%s,line:%d,size:%d\n", func, line, Count * Size);
	} else {
		ptr = tmemorydata(ptr, Count * Size);
		if (tmemory.total < MAXMLLOCNUM) {
			strncpy(tmemory.var[tmemory.total].func, func, STRINGLEN);
			tmemory.var[tmemory.total].line = line;
			tmemory.var[tmemory.total].size = Count * Size;
			tmemory.var[tmemory.total].ptr = ptr;
			tmemory.var[tmemory.total].num = tmemory.total;
		} else {
			printf("TERR: warning calloc num :%d,func:%s,line:%d\n", tmemory.total, func, line);
		}
		tmemory.total++;
		tmemory.lost++;
	}
	return ptr;
}

void   *trealloc(char *func, int line, void *Memory, size_t NewSize)
{
	void *ptr = NULL;
	if (Memory) {
		int num = -1;
		void *ptr = tmemoryflow(Memory, &num);
		if (!ptr) {
			printf("TERR: calloc err:func:%s,line:%d,size:%d,must call TALIGNED_FREE:func:%s,line:%d\n",
				func, line, NewSize,
				tmemory.var[num].func,
				tmemory.var[num].line);
			FREE((void *)(((unsigned long long *)Memory)[-1 - FLOWSIZE / 4]));
		}

		Memory = ptr;
	}

	ptr = REALLOC(Memory, NewSize + EXTRADATALEN);
	if (!ptr) {
		printf("TERR: calloc err:func:%s,line:%d,size:%d\n", func, line, NewSize);
	} else {
		ptr = tmemorydata(ptr, NewSize);
		if (tmemory.total < MAXMLLOCNUM) {
			strncpy(tmemory.var[tmemory.total].func, func, STRINGLEN);
			tmemory.var[tmemory.total].line = line;
			tmemory.var[tmemory.total].size = NewSize;
			tmemory.var[tmemory.total].ptr = ptr;
			tmemory.var[tmemory.total].num = tmemory.total;
		} else {
			printf("TERR: warning realloc num ：%d,func:%s,line:%d\n", tmemory.total, func, line);
		}
		tmemory.total++;
		tmemory.lost++;
	}
	return ptr;
}

void *tmalloc(char *func, int line, int size)
{
	void *ptr = MALLOC(size + EXTRADATALEN);
	if (!ptr) {
		printf("TERR: malloc err:func:%s,line:%d,size:%d\n", func, line, size);
	} else {
		ptr = tmemorydata(ptr, size);
		if (tmemory.total < MAXMLLOCNUM) {
			strncpy(tmemory.var[tmemory.total].func, func, STRINGLEN);
			tmemory.var[tmemory.total].line = line;
			tmemory.var[tmemory.total].size = size;
			tmemory.var[tmemory.total].ptr = ptr;
			tmemory.var[tmemory.total].num = tmemory.total;
		} else {
			printf("TERR: warning malloc num :%d,func:%s,line:%d\n", tmemory.total, func, line);
		}
		tmemory.total++;
		tmemory.lost++;
	}
	return ptr;
}

void tfree(void *palignedmem)
{
	if (palignedmem) {
		int num = -1;
		void *ptr = tmemoryflow(palignedmem, &num);
		if (ptr == NULL) {
			printf("TERR: TALIGNED_MALLOC must call TALIGNED_FREE:func:%s,line:%d\n",
				tmemory.var[num].func,
				tmemory.var[num].line);
			FREE((void *)(((unsigned long long *)palignedmem)[-1 - FLOWSIZE / 4]));
			return;
		}
		FREE(ptr);
	}
}

void *taligned_malloc(char *func, int line, int size, int alignment)
{
	if (alignment & (alignment - 1)) {
		return NULL;
	} else {
		void *praw = malloc(sizeof(void *) + EXTRADATALEN + size + alignment);
		if (praw) {
			unsigned char *pchar = praw;
			void *pbuf = (void *)(pchar + sizeof(void *) + EXTRADATALEN);
			void *palignedbuf = (void *)((((unsigned long long)pbuf) | (alignment - 1)) + 1);
			tmemorydata((void *)((unsigned long long)palignedbuf - FLOWSIZE), size);
			((unsigned long long *)palignedbuf)[-1 - FLOWSIZE / 4] = praw;
			if (tmemory.total < MAXMLLOCNUM) {
				strncpy(tmemory.var[tmemory.total].func, func, STRINGLEN);
				tmemory.var[tmemory.total].line = line;
				tmemory.var[tmemory.total].size = size;
				tmemory.var[tmemory.total].ptr = palignedbuf;
				tmemory.var[tmemory.total].num = tmemory.total;
				tmemory.var[tmemory.total].aligned = 1;
			} else {
				printf("TERR: warning taligned_malloc num ： %d,func:%s,line:%d\n", tmemory.total, func,
					line);
			}
			tmemory.total++;
			tmemory.lost++;
			return palignedbuf;
		} else {
			return NULL;
		}
	}
}

void taligned_free(void *palignedmem)
{
	if (palignedmem) {
		int num = -1;
		void *ptr = tmemoryflow(palignedmem, &num);
		if (tmemory.total > MAXMLLOCNUM) {
			printf("TERR: warning total:%d >num:%d\n", tmemory.total, MAXMLLOCNUM);
		} else {
			if (ptr) {
				printf("TERR: TMALLOC must call TFREE:func:%s,line:%d\n",
					tmemory.var[num].func,
					tmemory.var[num].line);
				FREE(ptr);
				return;
			}

		}
		FREE((void *)(((unsigned long long *)palignedmem)[-1 - FLOWSIZE / 4]));
	}
}
#endif
