#ifndef _MEMORYPOOL_H
#define _MEMORYPOOL_H

#include<iostream>
using namespace std;

enum { ALIGN = 8}; //小型区块的上调边界
enum { MAX_BYTES = 128}; //小型区块的上限
enum { NUMFREELISTS = MAX_BYTES / ALIGN};  //free-lists个数

//内存池
class Alloc{
public:
	static void *allocate(size_t n);   //分配指定大小的内存
	static void deallocate(void *p, size_t n);  //回收指定内存
	static void *reallocate(void *p, size_t old_sz, size_t new_sz);  //重新分配内存

private:
	union obj{   //free-lists节点构造
		union obj *free_list_link;
		char client_data[1];
	};

private:
	//16个free-lists
	static obj * volatile free_list[NUMFREELISTS];

	//以下函数根据区块的大小，决定使用第n号free-lists. n从0算起
	static size_t FREELIST_INDEX(size_t bytes){
		return ((bytes+ALIGN - 1) / ALIGN - 1);
	}

	//将bytes上调至8的倍数
	static size_t ROUND_UP(size_t bytes){
		return (((bytes)+ALIGN - 1) & ~(ALIGN - 1));
	}

	//返回一个大小为n的对象，并可能加入大小为n的其他区块到free-list
	static void *refill(size_t n);
	
	//配置一大块空间，可容纳nobjs个大小为size的区块
	//如果配饰nobjs个区块无法满足，nobjs可能会降低
	static char *chunk_alloc(size_t size, int &nobjs);

	//区块状态
	static char* start_free;    //内存池起始位置
	static char* end_free;      //内存池结束位置
	static size_t heap_size;
};

#endif