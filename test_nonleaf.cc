#include "stdio.h"
#include "BTreeNode.h"

int test_insert() {
    RC status;

    BTNonLeafNode node;

    int key = 0;
    PageId pid = 0;

    //node.locateChildPtr(searchKey, pid);
    //printf("pid is %i\n", pid);

    //status = node.insert(searchKey, pid);
    //printf("insert() returned %i\n", status);

    for (; key < 10; ++key, ++pid) {
        node.insert(key*10, pid);
    }

    node.insert(95, 11);
    node.showEntriesWithFirstPageId();

    PageFile pf;
    status = pf.open("nonleaf.txt",'w');
    printf("pf.open returned %i\n", status);
    status = node.write(0,pf);
    printf("write returned: %i\n", status);

    BTNonLeafNode newNode;
    status = newNode.read(0,pf);
    newNode.showEntriesWithFirstPageId();

}

int test_root(){
    BTNonLeafNode root;
    
    PageId pid1 = 5;
    PageId pid2 = 1;
    int key = 300;
    root.initializeRoot(pid1, key, pid2);
    root.showEntriesWithFirstPageId();
}

int test_insertAndSplit(){
    RC status;
    BTNonLeafNode left;
    BTNonLeafNode right;
        
    left.initializeRoot(1,10,2);
    left.insert(20, 3);
    left.showEntriesWithFirstPageId();
    right.showEntriesWithFirstPageId();

    int midkey = -1;
    PageId pid = 11;
    left.insertAndSplit(25, pid, right, midkey);

    printf("\nAFTER SPLIT\n");
    left.showEntriesWithFirstPageId();
    right.showEntriesWithFirstPageId(); 
}

int test_locateChildPtr(){
    RC status = 0;
    BTNonLeafNode test;

    test.initializeRoot(1,10,2);
    test.insert(20,3);
    test.insert(30, 4);   
    test.showEntriesWithFirstPageId();

    PageId pid = -1;
    test.locateChildPtr(5, pid);
    if(pid == 1)
        printf("Smaller than all serachKey test succesful.\n");
    else
        printf("smaller test failed.\n");
    test.locateChildPtr(25, pid);               
    if(pid == 3)
        printf("Mid sized searchKey test succesful.\n");
    else
        printf("test mid locate failed\n");
    test.locateChildPtr(500, pid);
    if(pid == 4)
        printf("Larger than all searchKey test successful.\n");
    else
        printf("test largest locate failed.\n");
}
int main() {
    printf("\n\n");
    //TODO: key is 0 tests
    //test_insert();
    //test_root();
    //test_insertAndSplit();
    test_locateChildPtr();
    
    printf("\n\n");
}
