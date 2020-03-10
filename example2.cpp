#include <cstdio>
#include "json.h"
#include "test.hpp"

void readValues(int* i) {
	printf("Read integer value %d\n", *i);
}

void readDates(char*& str) {
	int h,m,s;
  const char* i = str;
	while(*i!=0 && *i!='T') ++i;
	if(!*i) goto FAIL;
	if(sscanf(i+1, "%d:%d:%d", &h, &m, &s)==3) printf("Read time: %d:%d:%d\n", h, m, s);
	return;
	FAIL: puts("unknown time format");
}

int main() {
	JSONList json;

	try {
		json = JSON::parse(input);
		JSONList* sub = json.xpath("results/0/series/0/values");
		JSONList* dates = sub->xpath("[0]");
		JSONList* values = sub->xpath("[1]");
		values->iterate(readValues);
		dates->iterate(readDates);
	}
	catch(const char* msg) {
		puts(msg);
		puts(JSON::getState());
	}

}