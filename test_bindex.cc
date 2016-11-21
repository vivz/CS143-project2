#include "BTreeNode.h"
#include "BTreeIndex.h"
#include "string.h"
#include <stdio.h>
#include <string.h>
#include <vector>
#include <iostream>

using namespace std;

int test_locate(BTreeIndex index){
    printf("\n TEST LOCATE() \n");

    // manually create several nodes which will compose the test index
    RC status;
    IndexCursor cursor;

    // TODO: test locate() on an existing key
    printf("\n * * * * * * * * * * * * * * * * * * * * * * * * \n");
    cursor.pid = -1;
    cursor.eid = 0;
    printf("the key 70 is located at {pid: 6, eid: 0}\n");
    status = index.locate(70, cursor);
    if (status == 0)
        printf("key 70 at {pid: %i, eid: %i}\n", cursor.pid, cursor.eid);
    else
        printf("locate() failed!!!!! Error code %i\n", status);
    
    cursor.pid = -1;
    cursor.eid = 0;   
    status = index.locate(55, cursor);
    printf("the key 55 does not exist!\n");
    if (status == 0)
        printf("found key 55 at {pid: %i, eid: %i}\n", cursor.pid, cursor.eid);
    else if (status == RC_NO_SUCH_RECORD)
        printf("locate() failed with RC_NO_SUCH_RECORD\n");
    else 
        printf("locate() failed with error code %i\n", status);
}

int test_insert(BTreeIndex index) {
    printf("\n\nTEST INSERT\n\n");
    RC status;
    
    //Insert with no overflow
    IndexCursor cursor;
    {
        int key = 60;
        RecordId rid;
        rid.pid = 8;
        rid.sid = 60;

        status = index.insert(key, rid);
        printf("insert 60 returned %i\n", status);
        status = index.insert(55, rid);
        printf("insert 55 returned %i\n", status);
        status = index.insert(52, rid);
        printf("insert 52 returned %i\n", status);
        status = index.insert(25, rid);
        printf("insert 25 returned %i\n", status);
        
        cursor.pid = 1;
        cursor.eid = 0;
        status = index.locate(key, cursor);
        printf("locate() returned %i\n", status);
        printf("found key 60 at {pid: %i, eid: %i}\n", cursor.pid, cursor.eid);
        {
            int key;
            RecordId rid;
            status = index.readForward(cursor, key, rid);
            printf("readForward() returned %i: key: %i, rid: {pid: %i, sid: %i} \n", 
                                        status, key, rid.pid, rid.sid);
        }

        cursor.pid = 1;
        cursor.eid = 0;
        status = index.locate(55, cursor);
        printf("locate() returned %i\n", status);
        printf("found key 55 at {pid: %i, eid: %i}\n", cursor.pid, cursor.eid);
        {
            int key;
            RecordId rid;
            status = index.readForward(cursor, key, rid);
            printf("readForward() returned %i: key: %i, rid: {pid: %i, sid: %i} \n", 
                                        status, key, rid.pid, rid.sid);
        }   

        cursor.pid = 1;
        cursor.eid = 0;
        status = index.locate(52, cursor);
        printf("locate() returned %i\n", status);
        printf("found key 52 at {pid: %i, eid: %i}\n", cursor.pid, cursor.eid);
        {
            int key;
            RecordId rid;
            status = index.readForward(cursor, key, rid);
            printf("readForward() returned %i: key: %i, rid: {pid: %i, sid: %i} \n", 
                                        status, key, rid.pid, rid.sid);
        }

        cursor.pid = 1;
        cursor.eid = 0;
        status = index.locate(25, cursor);
        printf("locate() returned %i\n", status);
        printf("found key 25 at {pid: %i, eid: %i}\n", cursor.pid, cursor.eid);
        {
            int key;
            RecordId rid;
            status = index.readForward(cursor, key, rid);
            printf("readForward() returned %i: key: %i, rid: {pid: %i, sid: %i} \n", 
                                        status, key, rid.pid, rid.sid);
        }   

        cursor.pid = 1;
        cursor.eid = 0;
        status = index.locate(20, cursor);
        printf("locate() returned %i\n", status);
        printf("found key 20 at {pid: %i, eid: %i}\n", cursor.pid, cursor.eid);
        {
            int key;
            RecordId rid;
            status = index.readForward(cursor, key, rid);
            printf("readForward() returned %i: key: %i, rid: {pid: %i, sid: %i} \n", 
                                        status, key, rid.pid, rid.sid);
        }   

        cursor.pid = 1;
        cursor.eid = 0;
        status = index.locate(30, cursor);
        printf("locate() returned %i\n", status);
        printf("found key 30 at {pid: %i, eid: %i}\n", cursor.pid, cursor.eid);
        {
            int key;
            RecordId rid;
            status = index.readForward(cursor, key, rid);
            printf("readForward() returned %i: key: %i, rid: {pid: %i, sid: %i} \n", 
                                        status, key, rid.pid, rid.sid);
        }   

        cursor.pid = 1;
        cursor.eid = 0;
        status = index.locate(50, cursor);
        printf("locate() returned %i\n", status);
        printf("found key 50 at {pid: %i, eid: %i}\n", cursor.pid, cursor.eid);
        {
            int key;
            RecordId rid;
            status = index.readForward(cursor, key, rid);
            printf("readForward() returned %i: key: %i, rid: {pid: %i, sid: %i} \n", 
                                        status, key, rid.pid, rid.sid);
        }   

        cursor.pid = 1;
        cursor.eid = 0;
        status = index.locate(70, cursor);
        printf("locate() returned %i\n", status);
        printf("found key 70 at {pid: %i, eid: %i}\n", cursor.pid, cursor.eid);
        {
            int key;
            RecordId rid;
            status = index.readForward(cursor, key, rid);
            printf("readForward() returned %i: key: %i, rid: {pid: %i, sid: %i} \n", 
                                        status, key, rid.pid, rid.sid);
        }     

        cursor.pid = 1;
        cursor.eid = 0;
        status = index.locate(80, cursor);
        printf("locate() returned %i\n", status);
        printf("found key 80 at {pid: %i, eid: %i}\n", cursor.pid, cursor.eid);
        {
            int key;
            RecordId rid;
            status = index.readForward(cursor, key, rid);
            printf("readForward() returned %i: key: %i, rid: {pid: %i, sid: %i} \n", 
                                        status, key, rid.pid, rid.sid);
        }   

        cursor.pid = 1;
        cursor.eid = 0;
        status = index.locate(90, cursor);
        printf("locate() returned %i\n", status);
        printf("found key 90 at {pid: %i, eid: %i}\n", cursor.pid, cursor.eid);
        {
            int key;
            RecordId rid;
            status = index.readForward(cursor, key, rid);
            printf("readForward() returned %i: key: %i, rid: {pid: %i, sid: %i} \n", 
                                        status, key, rid.pid, rid.sid);
        }   
    }


    index.close();

}


int main() {
    RC status ;
    PageFile pf;
    string index_filename = "test.idx";
    status = pf.open(index_filename,'w');
    int rootPid = 1;
    cout << "Page file open status: " << status << endl;
    cout << "---------------------------------" <<  endl;

    BTNonLeafNode root;
    root.initializeRoot(rootPid+1, 70, rootPid+2);
    status = root.write(rootPid,pf);
    cout << "BTNonLeafNode root write status: " << status << endl;

    BTNonLeafNode leftChild;
    leftChild.initializeRoot(rootPid+3,50,rootPid+4);
    status = leftChild.write(rootPid+1,pf);
    cout << "BTNonLeafNode leftChild write status: " << status << endl;
    
    BTNonLeafNode rightChild;
    rightChild.initializeRoot(rootPid+5,80,rootPid+6);
    status = rightChild.write(rootPid+2,pf);
    cout << "BTNonLeafNode rightChild write status: " << status << endl;

    BTLeafNode firstLeaf;
    RecordId rid;
    rid.pid = 99;
    rid.sid = 20;

    firstLeaf.insert(20,rid);
    firstLeaf.insert(30,rid);
    firstLeaf.setNextNodePtr(rootPid+4);
    status = firstLeaf.write(rootPid+3,pf);
    cout << "BTLeafNode firstLeaf write status: " << status << endl;

    BTLeafNode secondLeaf;
    secondLeaf.insert(50, rid);
    secondLeaf.setNextNodePtr(rootPid+5);
    status = secondLeaf.write(rootPid+4,pf);
    cout << "BTLeafNode secondLeaf write status: " << status << endl;
    
    BTLeafNode thirdLeaf;
    thirdLeaf.insert(70, rid);
    thirdLeaf.setNextNodePtr(rootPid+6);
    status = thirdLeaf.write(rootPid+5, pf);
    cout << "BTLeafNode thirdLeaf write status: " << status << endl;
    
    BTLeafNode fourthLeaf;
    fourthLeaf.insert(80, rid);
    fourthLeaf.insert(90, rid);
    fourthLeaf.setNextNodePtr(-1);
    status = fourthLeaf.write(rootPid+6, pf);
    cout << "BTLeafNode fourthLeaf write status: " << status << endl;

    int treeHeight = 2;
    // TODO: make the index be composed of these nodes.
    BTreeIndex index1;
    status = index1.open(index_filename, 'r');
    index1.setTreeHeight(treeHeight);
    cout << "BTindex1 open status: " << status << endl;

    BTreeIndex index2;
    status = index2.open(index_filename, 'w');
    index2.setTreeHeight(treeHeight);
    cout << "BTindex2 open status: " << status << endl;

    //-----------------------------------------------------//
    //------------------------------------------------------//
    //test_locate(index1);
    test_insert(index2);
}