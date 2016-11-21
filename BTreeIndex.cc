/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 
#include "BTreeIndex.h"
#include "BTreeNode.h"
#include <stdio.h>
#include <string.h>

using namespace std;

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
	//initialize them to meaningful values???

    rootPid = -1;
    treeHeight = 0;

    //copy rootPid, PageId to the buffer
    memset(buffer, 0, PageFile::PAGE_SIZE); 
	memcpy(buffer, &rootPid, sizeof(PageId)); 
	memcpy(buffer+sizeof(PageId), &treeHeight, sizeof(int));
}

/*
 * Open the index file in read or write mode.
 * Under 'w' mode, the index file should be created if it does not exist.
 * @param indexname[IN] the name of the index file
 * @param mode[IN] 'r' for read, 'w' for write
 * @return error code. 0 if no error
 */
RC BTreeIndex::open(const string& indexname, char mode)
{
    RC rc;

  	// open the index file
  	if ((rc = pf.open(indexname, mode)) < 0) 
  		return rc;
  	
	if(mode == 'r' || mode == 'w'){

		if( pf.read(0, buffer) < 0) {
			return RC_FILE_READ_FAILED;
		}
		memcpy(&rootPid, buffer, sizeof(PageId)); 
		memcpy(&treeHeight, buffer+sizeof(PageId), sizeof(int));
		printf("Read from %s:\nrootPid = %i, treeHeight = %i.\n", indexname.c_str(), rootPid, treeHeight);
		return 0;
	}
	else {
		return RC_INVALID_FILE_MODE;
	}
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
	printf("Writing to index file before closing:\nrootPid = %i, treeHeight = %i.\n", rootPid, treeHeight);
	RC rc = writeVariables();
	if(rc < 0)
		return rc;
    return pf.close();
}

RC BTreeIndex::printEntries(){
	RC rc =printEntriesHelper(rootPid, 0);
	if(rc<0){
		printf("Error in printEntries\n");
		return rc;
	}
}

RC BTreeIndex::printEntriesHelper(PageId current_pid, int level){
	RC rc;
	//base case
	if(level == treeHeight){
		BTLeafNode leafNode;
		rc = leafNode.read(current_pid, pf);
		if(rc < 0){
			printf("error reading the leafNode under current_pid\n" );
			return rc;
		}
		printf("LeafNode with pid %i------", current_pid);
		leafNode.showEntries();
		return 0;
	}
	//non leaf node 
	else{
		BTNonLeafNode nonLeafNode;
		rc = nonLeafNode.read(current_pid, pf);
		if(rc < 0){
			printf("error reading the nonLeafNode under current_pid\n" );
			return rc;
		}
		printf("nonLeafNode with pid %i=============", current_pid);
		nonLeafNode.showEntriesWithFirstPageId();
		PageId iterator = nonLeafNode.getFirstPid();
		int i = 0;
		while(iterator!= -1){
			//printf("iterator is %i, i is %i", iterator, i);
			rc = printEntriesHelper(iterator, level+1);
			if(rc < 0){
				printf("error calling next level\n" );
				return rc;
			}
			iterator = nonLeafNode.getNextPid(i++);
		}
		return 0;
	}
}

RC BTreeIndex::writeVariables()
{
	memcpy(buffer, &rootPid, sizeof(PageId));
	memcpy(buffer+sizeof(PageId), &treeHeight, sizeof(int));
	if( pf.write(0, buffer) < 0) {
		return RC_FILE_WRITE_FAILED;
	}
	else 
		return 0;
}


/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
	//index file is empty
	if (treeHeight== 0){
		
		BTLeafNode small;
		PageId small_pid = pf.endPid();
		if(small.write(small_pid, pf))
			return RC_FILE_WRITE_FAILED;

		BTLeafNode big;
		PageId big_pid = pf.endPid();
		big.insert(key, rid);
		if(big.write(big_pid, pf)<0)
			return RC_FILE_WRITE_FAILED;

		BTNonLeafNode root;
		rootPid = pf.endPid();
		root.initializeRoot(small_pid, key, big_pid);
		if( root.write(rootPid, pf)<0)
			return RC_FILE_WRITE_FAILED;

		treeHeight = 1;

		return writeVariables();
	}
	else{ 

		bool has_overflow = false;
		NonLeafEntry overflow;
		LeafEntry toInsert;
		toInsert.key = key;
		// valid assignment???
		toInsert.rid = rid;

		RC rc = insertNonLeaf(toInsert, rootPid, 0, overflow, has_overflow);
		if(rc < 0)
			return rc;

		//new root
		if(has_overflow){
			BTNonLeafNode root;
			root.initializeRoot(rootPid, overflow.key, overflow.pid);
			PageId new_pid = pf.endPid();
			if(root.write(new_pid, pf)< 0)
				return RC_FILE_WRITE_FAILED;
			rootPid = new_pid;

			treeHeight++;
			return writeVariables();
		}
		return 0;
	}
}

RC BTreeIndex::insertNonLeaf(LeafEntry toInsert, PageId current_pid, int level, NonLeafEntry& overflow, bool& has_overflow){
	BTNonLeafNode node;
	node.read(current_pid, pf);
	RC rc = 0;

	//base case: leaf nodes
	if(level == treeHeight){
		rc = insertLeaf(toInsert, current_pid, overflow, has_overflow);
		if(rc!=0)
			return rc;
	}
	//recursing on non-leaf nodes
	else{
		PageId childPid = -1;
		node.locateChildPtr(toInsert.key, childPid);
		rc = insertNonLeaf(toInsert, childPid, level+1, overflow, has_overflow);
		if(rc!=0)
			return rc;
	}
	//if overflow is returned, insert new entry at the current level
	if(has_overflow){
		//need to overflow one level up
		if(node.isFull()){
			BTNonLeafNode sibling;
			has_overflow = true;
			int midKey = -1;
			PageId new_pid = pf.endPid();

			node.insertAndSplit(overflow.key, overflow.pid, sibling, midKey);

			overflow.key=midKey;
			overflow.pid= new_pid;//sibling's pid

			if(sibling.write(new_pid, pf)!=0)
				return RC_FILE_WRITE_FAILED;
		}
		//doesn't overflow at this level
		else{
			node.insert(overflow.key, overflow.pid);
			has_overflow = false;
		}

		if(node.write(current_pid,pf)!=0)
			return RC_FILE_WRITE_FAILED;
	}
	return 0;
}

RC BTreeIndex::insertLeaf(LeafEntry LE, PageId leafId, NonLeafEntry& overflow, bool& has_overflow){
	BTLeafNode leafNode;
	leafNode.read(leafId, pf);

	if(!leafNode.isFull()) {
		leafNode.insert(LE.key, LE.rid);
		has_overflow = false;
	}else{
		BTLeafNode sibling;
		PageId original_next = leafNode.getNextNodePtr();
		PageId new_pid = pf.endPid();
		overflow.pid = new_pid;
		has_overflow = true;

		leafNode.insertAndSplit(LE.key, LE.rid, sibling, overflow.key);

		leafNode.setNextNodePtr(new_pid);
		sibling.setNextNodePtr(original_next);
		if(sibling.write(new_pid, pf)!=0)
			return RC_FILE_WRITE_FAILED;
	}

	if(leafNode.write(leafId, pf))
		return RC_FILE_WRITE_FAILED;

	return 0;
}

/**
 * Run the standard B+Tree key search algorithm and identify the
 * leaf node where searchKey may exist. If an index entry with
 * searchKey exists in the leaf node, set IndexCursor to its location
 * (i.e., IndexCursor.pid = PageId of the leaf node, and
 * IndexCursor.eid = the searchKey index entry number.) and return 0.
 * If not, set IndexCursor.pid = PageId of the leaf node and
 * IndexCursor.eid = the index entry immediately after the largest
 * index key that is smaller than searchKey, and return the error
 * code RC_NO_SUCH_RECORD.
 * Using the returned "IndexCursor", you will have to call readForward()
 * to retrieve the actual (key, rid) pair from the index.
 * @param key[IN] the key to find
 * @param cursor[OUT] the cursor pointing to the index entry with
 *                    searchKey or immediately behind the largest key
 *                    smaller than searchKey.
 * @return 0 if searchKey is found. Othewise an error code
 */
RC BTreeIndex::locate(int searchKey, IndexCursor& cursor)
{
	return locateHelper(searchKey, rootPid, 0, cursor);
}


RC BTreeIndex::locateHelper(int key, PageId current_pid, int level, IndexCursor& cursor){
	if(level == treeHeight){
		int eid = -1;
		BTLeafNode leafNode;
		leafNode.read(current_pid, pf);

		RC rc = leafNode.locate(key, eid);

		if(rc!=0)
			return rc;

		cursor.eid=eid;
		cursor.pid=current_pid;
		return 0;
	}
	else{
		BTNonLeafNode node;
		RC rc2 = node.read(current_pid, pf);
		if(rc2<0)
			return rc2;

		PageId childPid = -1;
		node.locateChildPtr(key, childPid);
		return locateHelper(key, childPid, level+1, cursor);
	}
}



/*
 * Read the (key, rid) pair at the location specified by the index cursor,
 * and move foward the cursor to the next entry.
 * @param cursor[IN/OUT] the cursor pointing to an leaf-node index entry in the b+tree
 * @param key[OUT] the key stored at the index cursor location.
 * @param rid[OUT] the RecordId stored at the index cursor location.
 * @return error code. 0 if no error
 */
RC BTreeIndex::readForward(IndexCursor& cursor, int& key, RecordId& rid)
{
	BTLeafNode current;

	RC rc = current.read(cursor.pid, pf);
	if(rc < 0)
		return rc;
	
	RC rc2 = current.readEntry(cursor.eid, key, rid);
	if(rc2 < 0)
		return rc2;

	if(cursor.eid + 1 < current.getMaxKeyCount()){
		cursor.eid += 1;
	}
	else{
		cursor.pid = current.getNextNodePtr();
		cursor.eid = 0;
	}
    return 0;
}
