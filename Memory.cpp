// 
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
using namespace std;

#include "tmemory.h"
#include "memprotect.h"

void test(void)
{
	int num;
	int *ptr0 = NULL;
	int *ptr1 = NULL;
	int *ptr2 = NULL;

	//init
	TINIT();

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

    // deinit
	TDEINIT();
}

void test1() {
	//init
	TINIT();

	char * p = pmalloc(10);
	for (int i = 0; i < 10; i++) {
		p[i] = 'a' + i;
	}
	p[9] = 0;
	//p[10] = 0;
	//p[-1] = 10;
	printf("%s\n", p);
	pfree(p);

	p = pmalloc(10);
	for (int i = 0; i < 10; i++) {
		p[i] = 'a' + i;
	}
	p[9] = 0;
	//p[10] = 0;
	//p[-1] = 10;
	printf("%s\n", p);
	pfree(p);


    // deinit
	TDEINIT();
}

int main() {
	printf("===============================\n");
	test();
	printf("===============================\n");
	test1();

	return 0;
}