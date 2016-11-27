#include "Bruinbase.h"
#include "SqlEngine.h"
#include "string.h"
#include "stdio.h"
using namespace std;

int main(){
	RC rc;
	rc = SqlEngine::load("xsmall","xsmall.del", true);
	if(rc < 0)
		printf("there is an error\n");
	SelCond a;
	a.attr = 1;
	a.comp = SelCond::LT;
	string a_value = "2500";
	a.value = (char*)a_value.c_str();
	std::vector<SelCond> cond;
	cond.push_back(a);

	SqlEngine::select(1, "xsmall", cond);
	return 0;
}