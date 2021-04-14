//memprotect
//

#include "tmemory.h"
#include "memprotect.h"

#ifdef _MINGW_
#include "windows.h"
void* aligned_malloc(size_t size, size_t alignment)
{
    if(alignment & (alignment - 1))
    {
        return nullptr;
    }
    else
    {
        void *praw = malloc(sizeof(void*) + size + alignment);
        if(praw)
        {
            void *pbuf = reinterpret_cast<void*>(reinterpret_cast<size_t>(praw) + sizeof(void*));
            void *palignedbuf = reinterpret_cast<void*>((reinterpret_cast<size_t>(pbuf) | (alignment - 1)) + 1);
            ((void**)(palignedbuf))[-1] = praw;
            return palignedbuf;
        }
        else
        {
           return nullptr;
        }
    }
}
 
void aligned_free(void *palignedmem)
{
    free(reinterpret_cast<void*>(((static_cast<void**>(palignedmem))[-1])));
}
#else
#include <unistd.h>
#include <iostream>
#include <malloc.h>
#include <sys/mman.h>
#endif

void *pmalloc(int size) {
#ifdef _MINGW_
	int pagesize = 4096;
	int size_d = size % pagesize;
	int size_c = size / pagesize;
	if (size_d > 0) size_c ++;
	int size_f = size_d > 0 ? pagesize - size_d : 0;//
#ifdef P_OVERFLOW
	size_f = 0;
#endif
	//char * buffer = (char*)memalign(pagesize, (size_c + 2) * pagesize);
	//char * buffer = (char*)aligned_malloc((size_c + 2) * pagesize, pagesize);
	char * buffer = (char*)TALIGNED_MALLOC((size_c + 2) * pagesize, pagesize);
	char * buffer_real = buffer + size_f + pagesize;
	int offset_size = 2 * sizeof(int);
	int * buffer_int = (int*)(buffer_real - offset_size);
	*buffer_int = size_f + pagesize;
	buffer_int ++;
	*buffer_int = size_c + 2;
	int r = 0;
	DWORD old;
	r = VirtualProtect(buffer + (size_c + 1) * pagesize, pagesize, PAGE_NOACCESS , &old);
	if (!r) {
		LPVOID lpMsgBuf;
		DWORD dw = errno;//GetLastError(); 
	
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0, NULL );

		printf("error1: %d %s\n", errno, lpMsgBuf);
		LocalFree(lpMsgBuf);
	}
	r = VirtualProtect(buffer, pagesize, PAGE_NOACCESS , &old);
	if (!r) {
		LPVOID lpMsgBuf;
		DWORD dw = errno;//GetLastError(); 
	
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0, NULL );

		printf("error2: %d %s\n", errno, lpMsgBuf);
		LocalFree(lpMsgBuf);
	}
	return buffer_real;
#else
	int pagesize = sysconf(_SC_PAGE_SIZE);
	int size_d = size % pagesize;
	int size_c = size / pagesize;
	if (size_d > 0) size_c ++;
	int size_f = size_d > 0 ? pagesize - size_d : 0;//
#ifdef P_OVERFLOW
	size_f = 0;
#endif
	//char * buffer = (char*)memalign(pagesize, (size_c + 2) * pagesize);
	char * buffer = (char*)TALIGNED_MALLOC((size_c + 2) * pagesize, pagesize);
	char * buffer_real = buffer + size_f + pagesize;
	int offset_size = 2 * sizeof(int);
	int * buffer_int = (int*)(buffer_real - offset_size);
	*buffer_int = size_f + pagesize;
	buffer_int ++;
	*buffer_int = size_c + 2;
	int r = 0;
	r = mprotect(buffer + (size_c + 1) * pagesize, pagesize, PROT_READ);
	if (r) {
		printf("error1: %d %s\n", errno, strerror(errno));
	}
	r = mprotect(buffer, pagesize, PROT_READ);
	if (r) {
		printf("error2: %d %s\n", errno, strerror(errno));
	}
	return buffer_real;
#endif
}
void pfree(void * ptr) {
#ifdef _MINGW_
	int offset_size = 2 * sizeof(int);
	int * buffer_int = (int*)((char*)ptr - offset_size);
	int head_size = *buffer_int;
	buffer_int ++;
	int page_size = *buffer_int;
	int pagesize = 4096;
	int r = 0;
	DWORD old;
	r = VirtualProtect(ptr - head_size, page_size * pagesize, PAGE_EXECUTE_READWRITE, &old);
	if (!r) {
		LPVOID lpMsgBuf;
		DWORD dw = errno;//GetLastError(); 
	
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0, NULL );

		printf("error3: %d %s\n", errno, lpMsgBuf);
		LocalFree(lpMsgBuf);
	}
	//free(ptr - head_size);
	//aligned_free(ptr - head_size);
	TALIGNED_FREE(ptr - head_size);
#else
	int offset_size = 2 * sizeof(int);
	int * buffer_int = (int*)((char*)ptr - offset_size);
	int head_size = *buffer_int;
	buffer_int ++;
	int page_size = *buffer_int;
	int pagesize = sysconf(_SC_PAGE_SIZE);
	int r = 0;
	mprotect(ptr - head_size, page_size * pagesize, PROT_READ | PROT_WRITE);
	if (r) {
		printf("error3: %d %s\n", errno, strerror(errno));
	}
	//free(ptr - head_size);
	TALIGNED_FREE(ptr - head_size);
#endif
}