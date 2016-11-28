#include "Bruinbase.h"
#include "SqlEngine.h"
#include "string.h"
#include "stdio.h"
using namespace std;

int main(){
	RC rc;

	//rc = SqlEngine::load("xsmall","xsmall.del", true);
	//rc = SqlEngine::load("small","small.del", true);
	//rc = SqlEngine::load("medium","medium.del", true);
	//rc = SqlEngine::load("large","large.del", true);
	//rc = SqlEngine::load("xlarge","xlarge.del", true);

	//if(rc < 0)
	//	printf("there is an error in LOAD\n");
	
	SelCond a;
	a.attr = 1;
	a.comp = SelCond::GT;
	string a_value = "4000";
	a.value = (char*)a_value.c_str();

	SelCond b;
	b.attr = 1;
	b.comp = SelCond::EQ;
	string b_value = "272";
	b.value = (char*)b_value.c_str();
/*
	SelCond c;
	c.attr = 2;
	c.comp = SelCond::NE;
	string c_value = "Baby Take a Bow";
	c.value = (char*)c_value.c_str();*/
	
	std::vector<SelCond> cond;
	
	cond.push_back(a);
	//cond.push_back(b);
	//cond.push_back(c);

	SqlEngine::select(3, "xsmall", cond);
	return 0;
}