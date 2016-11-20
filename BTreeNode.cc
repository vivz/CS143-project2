#include "BTreeNode.h"
#include <string.h>
#include <stdio.h>

using namespace std;

//BTLeafNode Constructor
//First 4 byte is the keycount
BTLeafNode::BTLeafNode() {
	//Clear up the buffer
	memset(buffer, 0, PageFile::PAGE_SIZE);
	//Set keyCount and maxKeyCount
	keyCount = 0;
	maxKeyCount = ((PageFile::PAGE_SIZE) - sizeof(PageId) - sizeof(int))/sizeof(LeafEntry);
}

/**Getter**/
int BTLeafNode::getMaxKeyCount() {
	return maxKeyCount;
}

bool BTLeafNode::isFull() {
	return (getKeyCount() >= maxKeyCount);
}

/**Test purpose copied from github**/
void BTLeafNode::showEntries() {
    int nKeys = getKeyCount();
    if (nKeys == 0) {
        printf("THE NODE IS EMPTY\n");
        return;
    }
    printf("\nHere are the entries in this node:\n");
    int i = 0;
    for(LeafEntry* current = (LeafEntry*) (buffer + sizeof(int)); i < nKeys; ++current, ++i) {

        int key = -1;
        RecordId rid;

        //TODO: check return value
        readEntry(i, key, rid);

        printf("element #%i: {key: %i, rid: {pid: %i, sid: %i}}\n", i, key, rid.pid, rid.sid);
        
    }
}


/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf) { 
	//read selected PageFile into buffer.
	RC rc = pf.read(pid, buffer);
	keyCount = getKeyCount();
	return rc; 
}
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::write(PageId pid, PageFile& pf) { 
	//write to the selected PageFile from the buffer.
	return pf.write(pid, buffer);
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount(){
	//int count = 0;
	/*
	LeafEntry *currentEntry = (LeafEntry*) buffer;
	while((count < maxKeyCount) && currentEntry->key != 0) {
		count++;
		currentEntry++;
	}*/
	int *keyCountPtr = (int*) buffer;
	return *keyCountPtr;
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid) {
	keyCount = getKeyCount();
	int maxKey = getMaxKeyCount();
	//Node is full
	if(isFull()) {
		return RC_NODE_FULL;
	}
	//Locate the right location to insert
	int eid = -1;
	locate(key, eid);
	//Start inserting part
	//Move right
	for(int i = keyCount; i > eid; --i) {
		LeafEntry *temp = (LeafEntry*)(buffer + sizeof(int)) + i;
		*temp = *(temp - 1);
	}
	//Insert new leaf entry
	//TODO: Make sure there's no edge cases
	//TODO: handle duplicate insertion
	LeafEntry *insertedEntry = (LeafEntry*)(buffer + sizeof(int)) + eid;
	insertedEntry->key = key;
	insertedEntry->rid = rid;

	//Update key count
	int *keyCountPtr = (int*) buffer;
    *keyCountPtr = ++keyCount;

	return 0;
}

/*
 * Insert the (key, rid) pair to the node
 * and split the node half and half with sibling.
 * The first key of the sibling node is returned in siblingKey.
 * @param key[IN] the key to insert.
 * @param rid[IN] the RecordId to insert.
 * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
 * @param siblingKey[OUT] the first key in the sibling node after split.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, 
                              BTLeafNode& sibling, int& siblingKey) {
	keyCount = getKeyCount();
	int maxKey = getMaxKeyCount();
	int splitPos = (keyCount + 1) / 2;
	//sibling must be empty
	if(sibling.getKeyCount() != 0) {
		return RC_INVALID_ATTRIBUTE;
	}
	//TODO: uncomment this for later
	//Not large enought to be split
	
	if(keyCount < maxKey) {
		return RC_INVALID_FILE_FORMAT;
	}
	//Get eid to be inserted
	int eid = -1;
	locate(key, eid);
	//Assigned siblingKey
	if(eid == splitPos) {
		//Insert key is at split point
		//insert it as the first entry in the sibling
		siblingKey = key;
	} else {
		//Else, the key at the splitPos is the first key in sibling
		siblingKey = ((LeafEntry*)(buffer + sizeof(int)) + splitPos)->key;
	}
	//Start insert everything after splitPos to sibling and clean the buffer
	for(int i = splitPos; i < keyCount; i++) {
		LeafEntry *temp = (LeafEntry*)(buffer + sizeof(int)) + i;
        sibling.insert(temp->key, temp->rid);
        //clean up temp 
        temp->key = 0;
        temp->rid.sid = -1;
        temp->rid.pid = -1;
        //Update keyCount
        int *keyCountPtr = (int*) buffer;
        int curCount = *keyCountPtr;
    	*keyCountPtr = curCount - 1;
	}
	//insert the node wanted to be inserted
	if (eid < splitPos) {
		insert(key, rid);
	} else {
		sibling.insert(key, rid);
	}

	return 0;
}

/**
 * If searchKey exists in the node, set eid to the index entry
 * with searchKey and return 0. If not, set eid to the index entry
 * immediately after the largest index key that is smaller than searchKey,
 * and return the error code RC_NO_SUCH_RECORD.
 * Remember that keys inside a B+tree node are always kept sorted.
 * @param searchKey[IN] the key to search for.
 * @param eid[OUT] the index entry number with searchKey or immediately
                   behind the largest key smaller than searchKey.
 * @return 0 if searchKey is found. Otherwise return an error code.
 */
RC BTLeafNode::locate(int searchKey, int& eid) {
	LeafEntry *temp = (LeafEntry*)(buffer + sizeof(int));
	//TODO: ++temp make sure it works correct
	for(eid = 0; eid < getKeyCount(); ++eid, ++temp) {
		if(temp->key == searchKey) {
			return 0;
		} else if(temp->key > searchKey) {
			//Since the nodes in B+ tree are sorted
			return RC_NO_SUCH_RECORD;
		}
	}
	return RC_NO_SUCH_RECORD;
}

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid){
	if(eid < 0 || eid > getKeyCount()) {
		return RC_NO_SUCH_RECORD;
	} else if(buffer==NULL) {
		//TODO: Make sure what should be returned here.
		return -1;
	} else {
		LeafEntry *ptrEntry = (LeafEntry*)(buffer + sizeof(int)) + eid;
		key = ptrEntry->key;
		rid = ptrEntry->rid;
		return 0;
	}
}

/*
 * Return the pid of the next slibling node.
 * @return the PageId of the next sibling node 
 */
PageId BTLeafNode::getNextNodePtr(){
	PageId *ptrPage = (PageId*)(buffer + sizeof(int) + maxKeyCount * sizeof(LeafEntry));
	return *ptrPage;
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid){
	//invalid pid
	if(pid < 0) {
		return RC_INVALID_PID;
	}
	PageId *ptrPage = (PageId*)(buffer + sizeof(int) + maxKeyCount * sizeof(LeafEntry));
	*ptrPage = pid;
	return 0;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/**Constructor**/
BTNonLeafNode::BTNonLeafNode() {
	//Clear up the buffer
	memset(buffer, 0, PageFile::PAGE_SIZE);
	//Set keyCount and maxKeyCount
	keyCount = 0;
	maxKeyCount = ((PageFile::PAGE_SIZE) - sizeof(PageId) - sizeof(NonLeafEntry) - sizeof(int)) / sizeof(NonLeafEntry);
}

//Check if the node is Full
bool BTNonLeafNode::isFull() {
	return (getKeyCount() >= maxKeyCount);
}

//Getter
int BTNonLeafNode::getMaxKeyCount() {
	return maxKeyCount;
}

/**Tes purpose **/
void BTNonLeafNode::showEntries() {
    //return;
    printf("\nHere are the (key,pid) pairs:\n");
    int nKeys = getKeyCount();
    int i = 0;
    for (NonLeafEntry* e = (NonLeafEntry*)(buffer+sizeof(PageId)+sizeof(int)); i < nKeys + 1; ++e, ++i) {
        printf("entry #%i: {key: %i, pid: %i}\n", i, e->key, e->pid);
    }
}

void BTNonLeafNode::showEntriesWithFirstPageId(){
    //return;
    PageId* ptr = (PageId *) (buffer + sizeof(int));
    printf("\nFirst pid: %i",*ptr);
    showEntries();
}



/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf) {
	RC rc = pf.read(pid, buffer);
	keyCount = getKeyCount();
	return rc; 
}
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf) {
	return pf.write(pid, buffer);
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount() {
	int *keyCountPtr = (int*) buffer;
	return *keyCountPtr;
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid) {
	keyCount = getKeyCount();
	//Check if the node is Full
	if(isFull()) {
		return RC_NODE_FULL;
	}
	NonLeafEntry *temp = (NonLeafEntry*)(buffer + sizeof(PageId) + sizeof(int));
	//Find the point to insert
	int pos;
	for(pos=0; pos < keyCount; ++pos, ++temp) {
		if (temp->key >= key)
            break;
	}

	//Move right
	for (int i = keyCount; i > pos; --i) {
        NonLeafEntry *cur = (NonLeafEntry*)(buffer + sizeof(PageId) + sizeof(int)) + i;
        *cur = *(cur - 1);
    }
    //Insert
    temp->key = key;
    temp->pid = pid;
    //Update key count
	int *keyCountPtr = (int*) buffer;
    *keyCountPtr = ++keyCount;

    return 0;
}

/*
 * Insert the (key, pid) pair to the node
 * and split the node half and half with sibling.
 * The middle key after the split is returned in midKey.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @param sibling[IN] the sibling node to split with. This node MUST be empty when this function is called.
 * @param midKey[OUT] the key in the middle after the split. This key should be inserted to the parent node.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey) {
	keyCount = getKeyCount();
	int splitPos = (keyCount + 1) / 2;

	insert(key, pid);

	NonLeafEntry *temp = (NonLeafEntry*)(buffer + sizeof(PageId) + sizeof(int));
    midKey = (temp + splitPos)->key;   

    sibling.initializeRoot((temp+splitPos)->pid, (temp+splitPos+1)->key, (temp+splitPos+1)->pid);
    //Remove midkey since it will be inserted to parent node later
    (temp + splitPos)->key = 0;
    (temp + splitPos)->pid = 0;
    //Delete this since it has already been inserted.
    (temp + splitPos + 1)->key = 0;
    (temp + splitPos + 1)->pid = 0;
    for(int i=splitPos+2; i<keyCount; i++){
        NonLeafEntry *cur = (NonLeafEntry*)(buffer + sizeof(PageId) + sizeof(int)) + i;
        sibling.insert(cur->key, cur->pid);
        
        //Clean up
        cur->key = 0;
        cur->pid = 0;
        //Update keyCount
        int *keyCountPtr = (int*) buffer;
        int curCount = *keyCountPtr;
    	*keyCountPtr = curCount - 1;
      }
   
    return 0; 
}

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid) {
	int numKeys = getKeyCount();
	int i = 0;
	NonLeafEntry *temp = (NonLeafEntry*)(buffer + sizeof(PageId) + sizeof(int));

	if (searchKey < temp->key) {
		//if the search key is smaller than the current key
		PageId *pidPtr = (PageId*) (buffer + sizeof(int));
		pid = *pidPtr;
        return 0;
	}
    //if the search is larger or equal to the current key
    for(i = 1; i < numKeys; i++) {
        if (searchKey < (temp + i)->key){
            pid = (temp+i-1)->pid;
            return 0;
        }
    }
    //searchKey is the largest over all
    pid = (temp + i-1)->pid;
    return 0;
}

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
//Root doesn't have the 4 byte keycount in the front
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2) {
	//Clean up
	memset(buffer, 0, PageFile::PAGE_SIZE);
	//Error occured, RARE
	if(buffer == NULL) {
        return -1;
	}
	//Set up
	//First pid
    PageId *ptr = (PageId*)(buffer + sizeof(int));
    *ptr = pid1;
    //Next
    NonLeafEntry *rootEntry = (NonLeafEntry*)(buffer+ sizeof(int) + sizeof(PageId));
    rootEntry->key = key;
    rootEntry->pid = pid2;
    keyCount = 1;
    int* keyCountPtr = (int*) buffer;
    *keyCountPtr = keyCount;
    return 0;
}
