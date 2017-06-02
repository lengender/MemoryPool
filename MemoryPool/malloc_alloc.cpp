#include"malloc_alloc.h"
#include<iostream>

void* malloc_alloc::oom_malloc(size_t n){
	void *result = 0;
	int i = 0, j = 0;
	for (i = 0; i < 100000; ++i){   //不停的尝试释放，配置，再释放，再配置
		j = 0;
		while (j < 100000) j++;

		result = malloc(n);
		if (result) return result;
	}

	std::cout << "malloc error!" << std::endl;
	return 0;
}

void *malloc_alloc::oom_realloc(void *p, size_t n){
	void *result = 0;
	int i = 0, j = 0;
	for (i = 0; i < 100000; ++i){   //不停的尝试释放，配置，再释放，再配置
		j = 0;
		while (j < 100000) j++;

		result = realloc(p, n);
		if (result) return result;
	}

	std::cout << "realloc error!" << std::endl;
	return 0;
}