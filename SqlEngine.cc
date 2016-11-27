/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <errno.h>
#include <limits>
#include "Bruinbase.h"
#include "SqlEngine.h"
#include "BTreeIndex.h"

using namespace std;

// external functions and variables for load file and sql command parsing 
extern FILE* sqlin;
int sqlparse(void);


RC SqlEngine::run(FILE* commandline)
{
  fprintf(stdout, "Bruinbase> ");

  // set the command line input and start parsing user input
  sqlin = commandline;
  sqlparse();  // sqlparse() is defined in SqlParser.tab.c generated from
               // SqlParser.y by bison (bison is GNU equivalent of yacc)

  return 0;
}

RC SqlEngine::select(int attr, const string& table, const vector<SelCond>& cond)
{
  RecordFile rf;   // RecordFile containing the table
  RecordId   rid;  // record cursor for table scanning

  //Declaration:
  BTreeIndex btree;
  IndexCursor indexCursor;

  RC     rc;
  int    key;     
  string value;
  int    count;
  int    diff;
  int    index;
  const string indexFile = table + ".idx";
  const char readMode = 'r';

  // open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }

  // scan the table file from the beginning
  count = 0;
  //open the index file in 'r'
  rc = btree.open(indexFile, readMode);
  //if the index file doesn't exist
  if (rc < 0) {
  
  no_btree:
    rid.pid = rid.sid = 0;
    while (rid < rf.endRid()) {
      if(cond.size()>0 || attr!=4 ){
      // read the tuple
        if ((rc = rf.read(rid, key, value)) < 0) {
          fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
          goto exit_select;
        }

        // check the conditions on the tuple
        for (unsigned i = 0; i < cond.size(); i++) {
          // compute the difference between the tuple value and the condition value
          switch (cond[i].attr) {
            case 1:
              diff = key - atoi(cond[i].value);
              break;
            case 2:
              diff = strcmp(value.c_str(), cond[i].value);
              break;
          }

          // skip the tuple if any condition is not met
          switch (cond[i].comp) {
            case SelCond::EQ:
              if (diff != 0) goto next_tuple;
              break;
            case SelCond::NE:
              if (diff == 0) goto next_tuple;
              break;
            case SelCond::GT:
              if (diff <= 0) goto next_tuple;
              break;
            case SelCond::LT:
              if (diff >= 0) goto next_tuple;
              break;
            case SelCond::GE:
              if (diff < 0) goto next_tuple;
              break;
            case SelCond::LE:
              if (diff > 0) goto next_tuple;
              break;
          }
        }
      }

      // the condition is met for the tuple. 
      // increase matching tuple counter
      count++;

      // print the tuple 
      switch (attr) {
        case 1:  // SELECT key
          fprintf(stdout, "%d\n", key);
          break;
        case 2:  // SELECT value
          fprintf(stdout, "%s\n", value.c_str());
          break;
        case 3:  // SELECT *
          fprintf(stdout, "%d '%s'\n", key, value.c_str());
          break;
      }

      // move to the next tuple
      next_tuple:
        ++rid;
    }
  }
  //if there exists the btree index
  else {
    //************************
    //Find the range for keys
    //************************
    int start_key = 0;
    int end_key = std::numeric_limits<int>::max();
    bool condition_on_value = false;

    for (int i = 0; i < cond.size(); i++) {
      //Only check for key
      if (cond[i].attr != 1) {
        condition_on_value = true;
        continue;
      }
      //Found index
      if (cond[i].comp == SelCond::EQ) {
        start_key = atoi(cond[i].value);
        end_key = atoi(cond[i].value);
      }
      else if (cond[i].comp == SelCond::GE){
        start_key = max(start_key, atoi(cond[i].value));
      }
      else if (cond[i].comp == SelCond::LE){
        end_key = min(end_key, atoi(cond[i].value));
      }
      else if (cond[i].comp == SelCond::GT){
        start_key = max(start_key, atoi(cond[i].value)+1);
      }
      else if (cond[i].comp == SelCond::LT){
        end_key = min(end_key, atoi(cond[i].value)-1);
      }
    }
    //finalize the conditions
    if(start_key > end_key){
        rc = RC_NO_SUCH_RECORD;
        btree.close();
        goto found_exit;
    }
    //if no constrains on key, no need to access the tree;
    if(start_key == 0 && end_key == std::numeric_limits<int>::max()) {
      goto no_btree;
    }

    //************************
    //locate the starting point 
    //************************
    //printf("start_key is %i, end_key is %i\n",start_key, end_key );

    RC status = 0;
    btree.locate(start_key, indexCursor);
    if(btree.isEmtpyLeaf(indexCursor.pid)){
      btree.readForward(indexCursor, key, rid);
    }

    //*************************
    //iterate untill the end point
    //*************************
    while(status == 0) {
      status = btree.readForward(indexCursor, key, rid);
      if(key > end_key)
        break;

      bool valid_tuple = true;

      //if there is a need to look at value
      if(attr == 2 || attr == 3 || condition_on_value){
        if ((rc = rf.read(rid, key, value)) < 0) {
          fprintf(stderr, "Error: while reading a tuple from table %s, %i\n", table.c_str(), rc);
          goto exit_select;
        }
        for (int i = 0; i < cond.size(); i++) {
          // compute the difference between the tuple value and the condition value
          if(cond[i].attr == 2){
              diff = strcmp(value.c_str(), cond[i].value);
              if((cond[i].comp==SelCond::LT && diff>=0)||
                (cond[i].comp==SelCond::LE && diff>0) || 
                (cond[i].comp==SelCond::GE && diff<0) ||
                (cond[i].comp==SelCond::GT && diff>=0) ||
                (cond[i].comp==SelCond::EQ && diff!=0) ||
                (cond[i].comp==SelCond::NE && diff==0)){
                  valid_tuple = false;
                  break;
              }
          }
        }
      }
      // the condition is met for the tuple.
      if(valid_tuple){
        count++;
        // print the tuple
        switch (attr) {
          case 1:  // SELECT key
            fprintf(stdout, "%d\n", key);
            break;
          case 2:  // SELECT value
            fprintf(stdout, "%s\n", value.c_str());
            break;
          case 3:  // SELECT *
            fprintf(stdout, "%d '%s'\n", key, value.c_str());
            break;
        }
      }
    }
    btree.close();
  }

  found_exit:
    // print matching tuple count if "select count(*)"
    if (attr == 4) {
      fprintf(stdout, "%d\n", count);
    }
    rc = 0;

  // close the table file and return
  exit_select:
    rf.close();
    return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
  // If the table file doesn't exist, create it.
  // If the table file does exist, append it.
  // Declaration:
    RC rc;
    BTreeIndex btree;
    RecordFile recordFile;
    RecordId recordId;
    string readLine;
    int key;
    string value;
    const string tableName = table + ".tbl";
    const string indexName = table + ".idx";
    const char writeMode = 'w';
    //INDEX?????

    //Open loadfile for reading using fstream
    ifstream tableFile(loadfile.c_str());
    //Deal with open failure
    if (!tableFile.is_open()) {
      cerr << "Cannot open " << loadfile << " Error number: " << strerror(errno) << endl;
      return errno;
    }
    //Open a table in write mode
    rc = recordFile.open(tableName, writeMode);

    if (rc < 0) {
      cerr << "Error occured while reading a tupe from table " << table << endl;
      return rc;
    }
    //Index is true
    if(index) {
      //Open the Btree index file with write mode
      rc = btree.open(indexName, writeMode);
      if (rc != 0) {
        cerr << "Error occured while opening the index file, Error code: " << rc << endl;
        return rc;
      }

      //Read lines from the load file
      while (getline(tableFile, readLine)) {
        if(parseLoadLine(readLine, key, value) != 0) {
          cerr << "Error occured while parsing tableFile. Error number: " << RC_FILE_WRITE_FAILED << endl;
          return RC_FILE_WRITE_FAILED;
        }
        if (recordFile.append(key, value, recordId) != 0) {
          cerr << "Error occured while appending a line to RecordFile. Error number: " << RC_FILE_WRITE_FAILED << endl;
          return RC_FILE_WRITE_FAILED;
        }

        //Insert into Btree
        rc = btree.insert(key, recordId);
        if (rc != 0) {
          cerr << "Error occured while inserting (key, rid) into Btree. Error code: " << rc << endl;
          return rc;
        }

      }
       btree.close();
    }
    //Index is false
    else {
      while (getline(tableFile, readLine)) {
       if(parseLoadLine(readLine, key, value) != 0) {
          cerr << "Error occured while parsing tableFile. Error number: " << RC_FILE_WRITE_FAILED << endl;
          return RC_FILE_WRITE_FAILED;
        }
        if (recordFile.append(key, value, recordId) != 0) {
          cerr << "Error occured while appending a line to RecordFile. Error number: " << RC_FILE_WRITE_FAILED << endl;
          return RC_FILE_WRITE_FAILED;
        }
      }
    }
    //close files
    recordFile.close();
    tableFile.close();
    return rc;
}

RC SqlEngine::parseLoadLine(const string& line, int& key, string& value)
{
    const char *s;
    char        c;
    string::size_type loc;
    
    // ignore beginning white spaces
    c = *(s = line.c_str());
    while (c == ' ' || c == '\t') { c = *++s; }

    // get the integer key value
    key = atoi(s);

    // look for comma
    s = strchr(s, ',');
    if (s == NULL) { return RC_INVALID_FILE_FORMAT; }

    // ignore white spaces
    do { c = *++s; } while (c == ' ' || c == '\t');
    
    // if there is nothing left, set the value to empty string
    if (c == 0) { 
        value.erase();
        return 0;
    }

    // is the value field delimited by ' or "?
    if (c == '\'' || c == '"') {
        s++;
    } else {
        c = '\n';
    }

    // get the value string
    value.assign(s);
    loc = value.find(c, 0);
    if (loc != string::npos) { value.erase(loc); }

    return 0;
 }
