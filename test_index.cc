#include "BTreeIndex.h"
#include "string.h"
#include "stdio.h"
using namespace std;

int main(){
	BTreeIndex btIndex;

	printf("welcome to test_index\n");
	
	btIndex.open("test1.idx", 'r');

	//read in from the record file
	RecordFile rf;
	RC rc = rf.open("movie.tbl", 'w');
	if(rc < 0)
		return rc;

	RecordId rid;
	const int numInserts = 8;
	int key;
    string value;

	for (int i = 0; i < numInserts; ++i) {
		rid.pid = (PageId) 0;
        rid.sid = i;

        rc = rf.read(rid, key, value);
        if(rc < 0)
			return rc;
		printf("prepping insert of {%i, '%s'}\n", key, value.c_str());

		rc = btIndex.insert(key, rid);
		printf("btIndex.insert() returned %i\n", rc);
	}

	btIndex.close();

	return 0;
}