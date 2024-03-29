SRC = main.cc SqlParser.tab.c lex.sql.c SqlEngine.cc BTreeIndex.cc BTreeNode.cc RecordFile.cc PageFile.cc 
HDR = Bruinbase.h PageFile.h SqlEngine.h BTreeIndex.h BTreeNode.h RecordFile.h SqlParser.tab.h

bruinbase: $(SRC) $(HDR)
	g++ -ggdb -o $@ $(SRC)

lex.sql.c: SqlParser.l
	flex -Psql $<

SqlParser.tab.c: SqlParser.y
	bison -d -psql $<

clean:
	rm -f bruinbase bruinbase.exe *.o *~ lex.sql.c SqlParser.tab.c SqlParser.tab.h test_leaf test_nonleaf test_index test_bindex test1.idx test_select xlarge.idx xlarge.tbl small.idx small.tbl medium.idx medium.tbl xsmall.tbl test_unsorted.tbl xsmall.idx large.idx large.tbl test_node test_unsorted.idx test_unsorted test.idx test2.idx test.tbl nonleaf.txt difference.txt


LEAF = test_leaf.cc SqlParser.tab.c lex.sql.c SqlEngine.cc BTreeIndex.cc BTreeNode.cc RecordFile.cc PageFile.cc 

test_leaf:
	g++ -ggdb -o $@ $(LEAF)

NONLEAF = test_nonleaf.cc SqlParser.tab.c lex.sql.c SqlEngine.cc BTreeIndex.cc BTreeNode.cc RecordFile.cc PageFile.cc 

test_nonleaf:
	g++ -ggdb -o $@ $(NONLEAF)

INDEX = test_index.cc SqlParser.tab.c lex.sql.c SqlEngine.cc BTreeIndex.cc BTreeNode.cc RecordFile.cc PageFile.cc 

test_index: test_index.cc
	g++ -ggdb -o $@ $(INDEX)

BINDEX = test_bindex.cc SqlParser.tab.c lex.sql.c SqlEngine.cc BTreeIndex.cc BTreeNode.cc RecordFile.cc PageFile.cc 

test_bindex: test_bindex.cc
	g++ -ggdb -o $@ $(BINDEX)

SELECT = test_select.cc SqlParser.tab.c lex.sql.c SqlEngine.cc BTreeIndex.cc BTreeNode.cc RecordFile.cc PageFile.cc 

test_select: test_select.cc
	g++ -ggdb -o $@ $(SELECT)
