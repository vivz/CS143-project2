Skip to content
This repository
Search
Pull requests
Issues
Gist
 @yiran29
 Watch 2
  Star 0
  Fork 0 lowellbander/CS143
 Code  Issues 12  Pull requests 0  Projects 0  Wiki  Pulse  Graphs
Branch: master Find file Copy pathCS143/vm-shared/bruinbase/test_index.cc
61e220b  on Dec 9, 2014
@lowellbander lowellbander able to save data to disk and read it back
2 contributors @lowellbander @agentakki
RawBlameHistory     
217 lines (176 sloc)  5.77 KB
#include "BTreeNode.h"
#include "BTreeIndex.h"
#include "string.h"
#include <stdio.h>
#include <vector>

using namespace std;

string suffix(int num) {
    num %= 10;
    switch (num) {
        case 1:
            return "st";
        case 2:
            return "nd";
        case 3:
            return "rd";
        default:
            return "th";
    }
}

int test_locate(){
    printf("\n TEST_LOCATE() \n");

    // manually create several nodes which will compose the test index

    RC status ;
    PageFile pf;
    string index_filename = "test.idx";
    int rootPid = 0;
    status = pf.open(index_filename,'w');
    
    BTNonLeafNode root;
    root.initializeRoot(1, 50, 2);
    status = root.write(rootPid,pf);

    BTNonLeafNode leftChild;
    leftChild.initializeRoot(3,25,4);
    status = leftChild.write(1,pf);
    
    BTNonLeafNode rightChild;
    rightChild.initializeRoot(5,75,6);
    status = rightChild.write(2,pf);

    BTLeafNode firstLeaf;
    RecordId rid;
    rid.pid = 99 ;
    rid.sid = 20;

    firstLeaf.insert(5,rid);
    firstLeaf.insert(10,rid);
    firstLeaf.setNextNodePtr(4);
    status = firstLeaf.write(3,pf);

    BTLeafNode secondLeaf;
    secondLeaf.insert(35, rid);
    secondLeaf.insert(45, rid);
    secondLeaf.setNextNodePtr(5);
    status = secondLeaf.write(4,pf);
    
    BTLeafNode thirdLeaf;
    thirdLeaf.insert(60, rid);
    thirdLeaf.insert(70, rid);
    thirdLeaf.setNextNodePtr(6);
    status = thirdLeaf.write(5, pf);
    
    BTLeafNode fourthLeaf;
    fourthLeaf.insert(80, rid);
    fourthLeaf.insert(100, rid);
    fourthLeaf.setNextNodePtr(-1);
    status = fourthLeaf.write(6, pf);

    // TODO: make the index be composed of these nodes.
    BTreeIndex index;
    index.open(index_filename, 'r');
    int height = 3;
    int newHeight = index.setTreeHeight(height);
    printf("the height of the tree is %i\n", newHeight);

    IndexCursor cursor;

    // TODO: test locate() on an existing key
    printf("\n * * * * * * * * * * * * * * * * * * * * * * * * \n");
    cursor.pid = rootPid;
    cursor.eid = 0;
    printf("the key 45 is located at {pid: 4, eid: 1}\n");

    vector<int> parents;
    status = index.locate(45, cursor, ROOT_DEPTH, parents);
    print_pids(parents);
    if (status == 0)
        printf("found key 45 at {pid: %i, eid: %i}\n", cursor.pid, cursor.eid);
    else
        printf("locate() failed with error code %i\n", status);

    printf("\n * * * * * * * * * * * * * * * * * * * * * * * * \n\n");

    // TODO: test locate() on a key which does not exist
    
    cursor.pid = rootPid;
    cursor.eid = 0;   
    parents.clear(); //empties the vector
    status = index.locate(55, cursor, ROOT_DEPTH, parents);

    if (status == 0)
        printf("found key 55 at {pid: %i, eid: %i}\n", cursor.pid, cursor.eid);
    else if (status == RC_NO_SUCH_RECORD)
        printf("locate() failed with RC_NO_SUCH_RECORD\n");
    else 
        printf("locate() failed with error code %i\n", status);
}

int test_simple_insert() {
    printf("\n\nTEST INSERT()\n\n");
    RC status;

    // TODO: create a new index
    BTreeIndex index;
    string index_filename = "simple_insertion_test.idx";
    remove(index_filename.c_str());
    index.open(index_filename, 'w');
    
    // TODO: insert a single value, test locate and readForward on it
    IndexCursor cursor;
    {
        int key = 123;
        RecordId rid;
        rid.pid = 42;
        rid.sid = 69;

        status = index.insert(key, rid);
        printf("insert() returned %i\n", status);
        
        cursor.pid = index.getRootPid();
        cursor.eid = 0;
        vector<PageId> parents;
        status = index.locate(key, cursor, ROOT_DEPTH, parents);
        
        printf("locate() returned %i\n", status);
    }

    {
        int key;
        RecordId rid;
        status = index.readForward(cursor, key, rid);
        printf("readForward() returned %i: key: %i, rid: {pid: %i, sid: %i} \n", 
                                                status, key, rid.pid, rid.sid);
    }

    index.close();

}

int test_multi_insert(unsigned how_many) {
    /* create an index and insert several values into it
     * then readForward to retrieve all of them
     * */
    printf("\n\nTEST MULTI-INSERT()\n\n");
    
    string index_filename = "multi_insertion_test.idx";
    {
        // initialize index
        RC status;
        BTreeIndex index;
        remove(index_filename.c_str());
        printf("calling open()\n");
        index.open(index_filename, 'w');

        int key = 100;
        RecordId rid;
        rid.pid = 1000;
        rid.sid = 1;

        // insert several values
        for (int i = 0; i < how_many; ++i) {
            status = index.insert(key, rid);
            printf("index.insert() returned %i for key: %i, rid: {pid: %i, sid: %i}\n",
                                                            status, key, rid.pid, rid.sid);
            cout << "Done with " << i+1 << suffix(i+1) << " insertion.\n" << endl;
            printf("= - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = - = \n");
            key++;
            rid.pid++;
            rid.sid++;
        }
        index.close();
    }
    // read the values with readForward
    {
        RC status;
        BTreeIndex index;
        //remove(index_filename.c_str());
        printf("calling open()\n");
        status = index.open(index_filename, 'w');
        printf("index.open() returned %i\n", status);

        int key = 100;
        IndexCursor cursor;
        cursor.pid = index.getRootPid();
        printf("root pid is %i\n", cursor.pid);
        vector<PageId> parents;
        status = index.locate(key, cursor, ROOT_DEPTH, parents);

        printf("cursor: {pid: %i, eid: %i}\n", cursor.pid, cursor.eid);


    }
}

int test_getKeyCount();

int main() {
    //test_locate();
    test_simple_insert();
    test_multi_insert(5);
}
