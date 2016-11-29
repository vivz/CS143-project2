Ni Zhang: niazhang@ucla.edu
Yiran Wang: yolandawang@ucla.edu

Page read optimization:
1.	We parse the condition in advance to detect if there exists any conflicts in all the
	statements that are AND'ed together. If that is the case, we return without printing anything or opening any recordfiles/btrees. For example select * from xsmall where key<500 and key > 2000 would not trigger any pageRead.

2. 	If a the conditions only involves NE on keys (or does not involve key at all, of course), we
	do not use or open the btree. Instead we iterate through the record file. For example select * from xsmall where key<>123 and key<>321

3. 	If a query can be answered by only information in the bTree, we do not open the .tbl
	recordFile at all (in addition to not reading any individual record). For example select count(*) from xsmall where value <> 'hello'

4. 	If a query only selects attributes count(*) and does not have any conditions, we only
	iterates the rid values in the recordFile from {pid=0, sid=0} to rf.endRid(). In this way we achieves 1 page read for queries like select count(*) from <tableName>.

5. 	We keep the maximum key and minimum key as private variables in the BTree. If the
	condition asks only for entries with keys out of the range, we return immediately without reading any nodes in the tree. For example if the tree has maximum key 2500, a query select * from xsmall where key > 2600 would not trigger more pageReads than the one that opens the tree. Same for select * from xsmall where key = 2601

Extra Libraries:
<limits>