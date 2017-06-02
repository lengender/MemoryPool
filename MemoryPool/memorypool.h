#ifndef _MEMORYPOOL_H
#define _MEMORYPOOL_H

#include<iostream>
using namespace std;

enum { ALIGN = 8}; //С��������ϵ��߽�
enum { MAX_BYTES = 128}; //С�����������
enum { NUMFREELISTS = MAX_BYTES / ALIGN};  //free-lists����

//�ڴ��
class Alloc{
public:
	static void *allocate(size_t n);   //����ָ����С���ڴ�
	static void deallocate(void *p, size_t n);  //����ָ���ڴ�
	static void *reallocate(void *p, size_t old_sz, size_t new_sz);  //���·����ڴ�

private:
	union obj{   //free-lists�ڵ㹹��
		union obj *free_list_link;
		char client_data[1];
	};

private:
	//16��free-lists
	static obj * volatile free_list[NUMFREELISTS];

	//���º�����������Ĵ�С������ʹ�õ�n��free-lists. n��0����
	static size_t FREELIST_INDEX(size_t bytes){
		return ((bytes+ALIGN - 1) / ALIGN - 1);
	}

	//��bytes�ϵ���8�ı���
	static size_t ROUND_UP(size_t bytes){
		return (((bytes)+ALIGN - 1) & ~(ALIGN - 1));
	}

	//����һ����СΪn�Ķ��󣬲����ܼ����СΪn���������鵽free-list
	static void *refill(size_t n);
	
	//����һ���ռ䣬������nobjs����СΪsize������
	//�������nobjs�������޷����㣬nobjs���ܻή��
	static char *chunk_alloc(size_t size, int &nobjs);

	//����״̬
	static char* start_free;    //�ڴ����ʼλ��
	static char* end_free;      //�ڴ�ؽ���λ��
	static size_t heap_size;
};

#endif