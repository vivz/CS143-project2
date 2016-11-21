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
	RC rc = rf.open("test.tbl", 'r');
	if(rc < 0)
		return rc;

	RecordId rid;
	rid.pid = (PageId) 0;
	RecordId last = rf.endRid();
	printf("last rid is %i, %i \n", last.pid, last.sid);
	const int numInserts = 4;
	int key;
    string value;

	for (int i = 0; i < numInserts; ++i) {
        rid.sid = i%9;
        if(rid.sid==8)
        	rid.pid++;

        rc = rf.read(rid, key, value);
        if(rc < 0){
        	printf("error read from .tbl %i, %i \n", rid.pid, rid.sid);
			return rc;
        }
		printf("prepping insert of {key: %i, rid:{pid: %i, sid: %i}}\n", key, rid.pid, rid.sid);

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