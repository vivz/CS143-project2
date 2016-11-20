#include "BTreeIndex.h"
#include "string.h"
#include "stdio.h"
using namespace std;



int main(){
	BTreeIndex btIndex;

	printf("welcome to test_index\n");
	
	btIndex.open("test1.idx", 'r');

	btIndex.close();

	return 0;
}