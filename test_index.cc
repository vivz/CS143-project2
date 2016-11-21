#include "BTreeIndex.h"
#include "BTreeNode.h"
#include "string.h"
#include "stdio.h"
using namespace std;

int main(){
	BTreeIndex btIndex;

	printf("welcome to test_index\n");
	
	btIndex.open("test1.idx", 'w');

	//read in from the record file
	RecordFile rf;
	RC rc = rf.open("test_unsorted.tbl", 'r');
	if(rc < 0)
		return rc;

	RecordId rid;
	const int numInserts = 1;
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

	//search
	/*

	IndexCursor ic;
	for(int searchkey = 1000; searchkey<1015; searchkey++){
		rc = btIndex.locate(searchkey, ic);
		if(rc<0){
			printf("searchkey %i not found\n", searchkey);
		}else{
			printf("entry %i is at pid: %i, eid: %i\n", searchkey, ic.pid, ic.eid);
		}
	}*/

	btIndex.printEntries();

	btIndex.close();

	return 0;
}