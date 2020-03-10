#include <cstdio>
#include "json.h"
#include "test.hpp"

int main() {
	try {
		JSON::parse(input).xpath("results/0/series/0/values/[1]")->printValue(true);
	}
	catch(const char* msg) {
		puts(msg);
		puts(JSON::getState());
	}

}