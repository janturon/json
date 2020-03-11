#include <cstdio>
#include "json.h"
#include "test.hpp"

int main() {
	try {
		JSONList* data = JSON::parse(input).xpath("results/0/series/0/values/[1]");
		for(auto i=(JSONList*)data->value; i!=NULL; i=i->next) {
			printf("Read value: %d\n", *(int*)i->value);
		}
	}
	catch(const char* msg) {
		puts(msg);
		puts(JSON::getState());
	}

}