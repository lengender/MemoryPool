#include"memorypool.h"
#include"malloc_alloc.h"

//static成员的初值设定
char *Alloc::start_free = 0;
char *Alloc::end_free = 0;
size_t Alloc::heap_size = 0;
Alloc::obj * volatile Alloc::free_list[NUMFREELISTS] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

//分配内存
void* Alloc::allocate(size_t n){
	obj *volatile *my_free_list;
	obj *result;

	//大于128就调用malloc_alloc
	if (n > 128)
		return malloc_alloc::allocate(n);

	//寻找16个free_list中最合适的一个
	my_free_list = free_list + FREELIST_INDEX(n);
	result = *my_free_list; 
	if (result == 0){
		//没有找到可用的free-list,准备重新填充free list
		void* r = refill(ROUND_UP(n)); 
		return r;
	}

	//调整free-list
	*my_free_list = result->free_list_link;
	return result;
}

//释放内存
void Alloc::deallocate(void *p, size_t n){
	obj *q = (obj*)p;
	obj *volatile *my_free_list;

	//如果大于128就调用
	if (n > 128){
		malloc_alloc::deallocate(p, n);
		return;
	}

	//寻找对应的区块
	my_free_list = free_list + FREELIST_INDEX(n);
	//调整free list, 回收区块
	q->free_list_link = *my_free_list;
	*my_free_list = q;
}

//返回一个大小为n的对象，并且有时候会为适当的free list增加节点
//假设n已经适当上调至8的倍数
void* Alloc::refill(size_t n){
	int nobjs = 3;
	//调用chunk_alloc(), 尝试取得nobjs个区块作为free list的新节点
	//注意参数nobjs是pass by reference
	char *chunk = chunk_alloc(n, nobjs);   
	obj* volatile *my_free_list;
	obj* result;
	obj* current_obj, *next_obj;
	int i;

	//如果只获得一个区块，这个区块就分配给调用者，free list无新节点
	if (nobjs == 1) return (chunk);

	//否则准备调整free_list, 纳入新节点
	my_free_list = free_list + FREELIST_INDEX(n);

	//以下是在chunk空间内建立free list
	result = (obj*)chunk;   //这一块准备返回给客户端

	//以下导引free list指向新配置的空间(取自内存池)
	*my_free_list = next_obj = (obj*)(chunk + n);
	
	//以下将free list的各节点串接起来
	for (i = 1;; i++){  //从1开始，因为第0块将返给客户端
		current_obj = next_obj;
		next_obj = (obj*)((char*)next_obj + n);
		if (nobjs - 1 == i){
			current_obj->free_list_link = 0;
			break;
		}
		else{
			current_obj->free_list_link = next_obj;
		}
	}

	return result;
}

//内存池配置内存
//调用chunk_alloc(), 尝试取得nobjs个区块作为free list的新节点
//注意参数nobjs是pass by reference
char * Alloc::chunk_alloc(size_t size, int &nobjs){
	char *result;
	size_t total_bytes = size * nobjs;
	size_t bytes_left = end_free - start_free;  //内存池剩余内存

	if (bytes_left >= total_bytes){
		//内存池剩余空间完全满足需求量
		result = start_free;
		start_free += total_bytes;
		return (result);
	}
	else if (bytes_left >= size){
		//内存池剩余空间不能完全满足需求量，但足够供应一个及其以上的区块
		nobjs = bytes_left / size;
		total_bytes = size * nobjs;
		result = start_free;
		start_free += total_bytes;
		return result;
	}
	else{
		//内存池剩余的空间连一个区块的大小都无法提供
		size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);
		
		//以下试着让内存池中的残余零头还有利用价值
		if (bytes_left > 0){
			//内存池内有一些零头，先配给适当的free_list
			//首先寻找适当的free-list
			obj *volatile *my_free_list = free_list + FREELIST_INDEX(bytes_left);
			//调整free_list, 将内存池中的剩余空间插入
			((obj*)start_free)->free_list_link = *my_free_list;
			*my_free_list = (obj*)start_free;
		}

		//配置heap空间，用来补充内存池
		start_free = (char*)malloc(bytes_to_get);
		if (0 == start_free){
			//heap空间不足，malloc失败
			int i;
			obj *volatile *my_free_list, *p;
			//以下搜寻适当的free_list
			//所谓适当是指“尚有未用区块，且区块够大”的free_list
			for (i = size; i <= MAX_BYTES; i += ALIGN){
				my_free_list = free_list + FREELIST_INDEX(i);
				p = *my_free_list;
				if (p != 0){  //free_list尚有未用区块
					//调整free_list以释放出未用区块
					*my_free_list = p->free_list_link;
					start_free = (char*)p;
					end_free = start_free + i;
					//递归调用自己，为了修正 nobjs;
					return (chunk_alloc(size, nobjs));
				}
			}

			end_free = 0;
			start_free = (char*)malloc_alloc::allocate(bytes_to_get);
		}

		heap_size += bytes_to_get;
		end_free = start_free + bytes_to_get;
		//递归调用自己，为了修正nobjs
		return (chunk_alloc(size, nobjs));
	}
}
