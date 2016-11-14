#include "BTreeNode.h"
#include "string.h"
#include "stdio.h"
using namespace std;

int test_leaf_insert_RW() {
    printf("\n");

    RC status;

    // make a new node

    BTLeafNode leaf;
    //leaf.showBuffer();

    // get a handle for the recordfile for small.tbl

    RecordFile rf;
    status = rf.open("movie.tbl", 'w');
    printf("rf.open() returned %i\n", status);

    // insert an entry into the node using the first record in the table

    int key;
    string value;

    RecordId rid;
    RecordId last = rf.endRid();
    printf("last rid is {pid: %i, sid: %i}\n", last.pid, last.sid);

    printf("\n--------------------------------------------------\n");

    const int numInserts = 10;

    for (int i = 0; i < numInserts; ++i) {
        printf("\n");

        rid.pid = (PageId) 0;
        rid.sid = i;

        status = rf.read(rid, key, value);
        printf("rf.read() returned %i\n", status);
        printf("prepping insert of {%i, '%s'}\n", key, value.c_str());

        status = leaf.insert(key, rid);
        printf("leaf.insert() returned %i\n", status);
       // leaf.showEntries();
        printf("\n--------------------------------------------------\n");
    }

    // somehow test that it was inserted? maybe by calling locate with the inserted key
    // and with a key that we have not inserted.

    // we can do more complete testing when we have implemented readEntry
    printf("\n");

    leaf.showEntries();

    // write the contents of a node to a pagefile and read it back into another
    // node.
    
    PageFile pf;
    status = pf.open("leaf.txt", 'w');
    printf("opened file for writing with status code %i\n", status);
    status = leaf.write(0, pf);
    printf("wrote to file with status %i\n", status);

    BTLeafNode anotherNode;
    status = anotherNode.read(0, pf);
    printf("opened file for reading with status code %i\n", status);
    anotherNode.showEntries();
    
    return 0;
}

int test_sibling() {
    printf("\n");

    RC status;

    // create two sibling nodes 
    BTLeafNode left;
    BTLeafNode right;
    PageId left_pid = 0;
    PageId right_pid = 5;
    
    // load half of a .del into one, the other half into another
    RecordFile rf;
    rf.open("movie.tbl", 'w');

    RecordId rid;
    rid.pid = 0;

    int key;
    string value;
    
    for (int i = 0; i < 8; ++i) {
        rid.sid = i;
        rf.read(rid, key, value);
        if (i < 4)
            left.insert(key, rid);
        else
            right.insert(key, rid);
    }

    // make right left's sibling, and save the nodes to a file

    left.setNextNodePtr(right_pid);

    PageFile pf;
    pf.open("sibling_test.txt", 'w');

    left.write(left_pid, pf);
    right.write(right_pid, pf);
    
    // load the nodes from memory
    
    BTLeafNode newLeft;
    BTLeafNode newRight;

    PageFile newPf;
    newPf.open("sibling_test.txt", 'r');

    newLeft.read(left_pid, newPf);

    // print one's contents, then follow the pointer, print the other

    newLeft.showEntries();

    PageId sib_pid = newLeft.getNextNodePtr();
    printf("sib pid is %i\n", sib_pid);

    newRight.read(sib_pid, newPf);
    newRight.showEntries();
    
    return 0;
}

int test_split(){
    BTLeafNode left;
    BTLeafNode right;
    PageId left_pid = 0;
    PageId right_pid = 5;

    //load all of a .del into one
    RecordFile rf;
    rf.open("movie.tbl",'w');

    RecordId rid;
    rid.pid = 0;
    
    int key;
    string value;

    for(int i = 0; i<84; ++i){
        rid.sid = i;
        rf.read(rid, key, value);
        left.insert(key,rid);
    }

    PageFile pf;
    pf.open("split_test.txt", 'w');
    left.write(left_pid, pf);
    left.showEntries();

    RecordId rid2;
    rid2.pid = 0;
    rid2.sid = 84;
    int sibKey = -5;
    //left.insertAndSplit(3229,rid2, right,sibKey);
    left.insertAndSplit(212,rid2, right,sibKey);

    left.showEntries();
    printf("sib key after split is %d\n", sibKey);
    right.showEntries();
    return 0;
}

int main() {
    test_leaf_insert_RW();
    printf("------------TEST insert END----------------------");
    test_sibling();
    printf("------------TEST sibling END----------------------");
    test_split();
    printf("------------TEST Split END----------------------");
}
