#ifndef MALLOC_ALLOC_H
#define MAlloc_ALLOC_H
#include<stdlib.h>

class malloc_alloc{
public:
	static void *allocate(size_t n){
		void *result = malloc(n);
		if (0 == result) result = oom_malloc(n);
		return result;
	}

	static void deallocate(void *p, size_t n){
		free(p);  //n其实完全没有用上
	}

	static void *reallocate(void *p, size_t old_sz, size_t new_sz){
		void *result = realloc(p, new_sz);   
		if (0 == result) result = oom_realloc(p, new_sz);
		return result;
	}

private:
	//以下函数用以处理内存不足的情况
	static void *oom_malloc(size_t n);
	static void *oom_realloc(void* p, size_t n);
};
#endif