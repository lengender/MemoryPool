#include"memorypool.h"
#include<vector>

int main(){
	Alloc pool;
	int *p =(int*)pool.allocate(sizeof(int) * 10);
	if (p == 0)
		cout << "allocate error." << endl;

	for (int i = 0; i < 10; ++i)
		p[i] = i;

	for (int i = 0; i < 10; ++i)
		cout << p[i] << " ";
	cout << endl;

	pool.deallocate(p, 40);

	return 0;
	
}
