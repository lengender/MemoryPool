#include"memorypool.h"
#include"malloc_alloc.h"

//static��Ա�ĳ�ֵ�趨
char *Alloc::start_free = 0;
char *Alloc::end_free = 0;
size_t Alloc::heap_size = 0;
Alloc::obj * volatile Alloc::free_list[NUMFREELISTS] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

//�����ڴ�
void* Alloc::allocate(size_t n){
	obj *volatile *my_free_list;
	obj *result;

	//����128�͵���malloc_alloc
	if (n > 128)
		return malloc_alloc::allocate(n);

	//Ѱ��16��free_list������ʵ�һ��
	my_free_list = free_list + FREELIST_INDEX(n);
	result = *my_free_list; 
	if (result == 0){
		//û���ҵ����õ�free-list,׼���������free list
		void* r = refill(ROUND_UP(n)); 
		return r;
	}

	//����free-list
	*my_free_list = result->free_list_link;
	return result;
}

//�ͷ��ڴ�
void Alloc::deallocate(void *p, size_t n){
	obj *q = (obj*)p;
	obj *volatile *my_free_list;

	//�������128�͵���
	if (n > 128){
		malloc_alloc::deallocate(p, n);
		return;
	}

	//Ѱ�Ҷ�Ӧ������
	my_free_list = free_list + FREELIST_INDEX(n);
	//����free list, ��������
	q->free_list_link = *my_free_list;
	*my_free_list = q;
}

//����һ����СΪn�Ķ��󣬲�����ʱ���Ϊ�ʵ���free list���ӽڵ�
//����n�Ѿ��ʵ��ϵ���8�ı���
void* Alloc::refill(size_t n){
	int nobjs = 3;
	//����chunk_alloc(), ����ȡ��nobjs��������Ϊfree list���½ڵ�
	//ע�����nobjs��pass by reference
	char *chunk = chunk_alloc(n, nobjs);   
	obj* volatile *my_free_list;
	obj* result;
	obj* current_obj, *next_obj;
	int i;

	//���ֻ���һ�����飬�������ͷ���������ߣ�free list���½ڵ�
	if (nobjs == 1) return (chunk);

	//����׼������free_list, �����½ڵ�
	my_free_list = free_list + FREELIST_INDEX(n);

	//��������chunk�ռ��ڽ���free list
	result = (obj*)chunk;   //��һ��׼�����ظ��ͻ���

	//���µ���free listָ�������õĿռ�(ȡ���ڴ��)
	*my_free_list = next_obj = (obj*)(chunk + n);
	
	//���½�free list�ĸ��ڵ㴮������
	for (i = 1;; i++){  //��1��ʼ����Ϊ��0�齫�����ͻ���
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

//�ڴ�������ڴ�
//����chunk_alloc(), ����ȡ��nobjs��������Ϊfree list���½ڵ�
//ע�����nobjs��pass by reference
char * Alloc::chunk_alloc(size_t size, int &nobjs){
	char *result;
	size_t total_bytes = size * nobjs;
	size_t bytes_left = end_free - start_free;  //�ڴ��ʣ���ڴ�

	if (bytes_left >= total_bytes){
		//�ڴ��ʣ��ռ���ȫ����������
		result = start_free;
		start_free += total_bytes;
		return (result);
	}
	else if (bytes_left >= size){
		//�ڴ��ʣ��ռ䲻����ȫ���������������㹻��Ӧһ���������ϵ�����
		nobjs = bytes_left / size;
		total_bytes = size * nobjs;
		result = start_free;
		start_free += total_bytes;
		return result;
	}
	else{
		//�ڴ��ʣ��Ŀռ���һ������Ĵ�С���޷��ṩ
		size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);
		
		//�����������ڴ���еĲ�����ͷ�������ü�ֵ
		if (bytes_left > 0){
			//�ڴ������һЩ��ͷ��������ʵ���free_list
			//����Ѱ���ʵ���free-list
			obj *volatile *my_free_list = free_list + FREELIST_INDEX(bytes_left);
			//����free_list, ���ڴ���е�ʣ��ռ����
			((obj*)start_free)->free_list_link = *my_free_list;
			*my_free_list = (obj*)start_free;
		}

		//����heap�ռ䣬���������ڴ��
		start_free = (char*)malloc(bytes_to_get);
		if (0 == start_free){
			//heap�ռ䲻�㣬mallocʧ��
			int i;
			obj *volatile *my_free_list, *p;
			//������Ѱ�ʵ���free_list
			//��ν�ʵ���ָ������δ�����飬�����鹻�󡱵�free_list
			for (i = size; i <= MAX_BYTES; i += ALIGN){
				my_free_list = free_list + FREELIST_INDEX(i);
				p = *my_free_list;
				if (p != 0){  //free_list����δ������
					//����free_list���ͷų�δ������
					*my_free_list = p->free_list_link;
					start_free = (char*)p;
					end_free = start_free + i;
					//�ݹ�����Լ���Ϊ������ nobjs;
					return (chunk_alloc(size, nobjs));
				}
			}

			end_free = 0;
			start_free = (char*)malloc_alloc::allocate(bytes_to_get);
		}

		heap_size += bytes_to_get;
		end_free = start_free + bytes_to_get;
		//�ݹ�����Լ���Ϊ������nobjs
		return (chunk_alloc(size, nobjs));
	}
}
