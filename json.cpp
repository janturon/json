#include <cstdio>
#include <cstring>
#include "json.h"

static char JSONbuffer[80];

// JSONList

JSONList::JSONList()
	: type('n'), key(NULL), value(NULL), next(NULL), select(NULL) {
		refs = new unsigned char;
		*refs = 1;
	}

JSONList::JSONList(const JSONList& copy) {
	type = copy.type;
	// visitor
	refs = copy.refs;
	valueRefInc();
  // shallow
	key = copy.key;
	value = copy.value;
	// unhook
	next = NULL;
}

JSONList& JSONList::operator=(const JSONList& rhs) {
	type = rhs.type;
	refs = rhs.refs;
	valueRefInc();
	key = rhs.key;
	value = rhs.value;
	return *this;
}

void JSONList::valueRefInc() {
	*refs+= 1;
  if(type=='a' || type=='o')
	for(JSONList* i=(JSONList*)value; i!=NULL; i=i->next) {
		i->valueRefInc();
	}
}

JSONList::~JSONList() {
	*refs-= 1;
	if(*refs) return;

	delete refs;
  if(key!=NULL) {	delete key; key = NULL;	}

	if(value!=NULL) {
		switch(type) {
			case 's': delete [] (char*)value; value = NULL; break;
			case 'i': delete (int*)value; value = NULL; break;
			case 'f': delete (float*)value; value = NULL; break;
			case 'o':
			case 'a': for(JSONList* i=(JSONList*)value; i!=NULL; ) {
				JSONList* next = i->next;
				delete i;
				i = next;
			}
		}
	}

	if(select!=NULL) delete select;
}

int JSONList::getInt() const { return *(int*)value;	}
float JSONList::getFloat() const { return *(float*)value; }
char* JSONList::getString() const { return (char*)value; }

void JSONList::iterate(void(*f)(void*,char)) {
	if(type!='o' && type!='a') throw "Can't iterate on non object.";
  for(JSONList* i=(JSONList*)value; i!=NULL; i=i->next) f(i->value, i->type);
}
void JSONList::iterate(void(*f)(const char*)) {
	if(type!='o' && type!='a') throw "Can't iterate on non object.";
  for(JSONList* i=(JSONList*)value; i!=NULL; i=i->next) f((const char*)i->value);
}
void JSONList::iterate(void(*f)(char*&)) {
	if(type!='o' && type!='a') throw "Can't iterate on non object.";
  for(JSONList* i=(JSONList*)value; i!=NULL; i=i->next) f((char*&)i->value);
}
void JSONList::iterate(void(*f)(int*)) {
	if(type!='o' && type!='a') throw "Can't iterate on non object.";
  for(JSONList* i=(JSONList*)value; i!=NULL; i=i->next) f((int*)i->value);
}
void JSONList::iterate(void(*f)(float*)) {
	if(type!='o' && type!='a') throw "Can't iterate on non object.";
  for(JSONList* i=(JSONList*)value; i!=NULL; i=i->next) f((float*)i->value);
}


void JSONList::printValue(bool all) const {
	switch(type) {
		case 's': printf("s(%s)", (char*)value); break;
		case 'i': printf("i(%d)", *(int*)value); break;
		case 'f': printf("f(%f)", *(float*)value); break;
		case 'a':
			if(all) {
				putchar('[');
				for(JSONList* i=(JSONList*)value; i!=NULL; i=i->next) {
					i->printValue(all);
					if(i->next!=NULL) putchar(',');
				}
				putchar(']');
			}
			else printf("%s", "[array]");
		break;
		case 'o':
			if(all) {
				putchar('{');
				for(JSONList* i=(JSONList*)value; i!=NULL; i=i->next) {
					printf("%s", i->key);
					putchar(':');
					i->printValue(all);
					if(i->next!=NULL) putchar(',');
				}
				putchar('}');
			}
			else printf("%s", "{object}");
		break;
		default: printf("%s", "[NULL]"); break;
	}
}

JSONList* JSONList::xpath(const char* path) {
	parsedPath = path;
	JSONList* result = this;
	int index, level=0;
	for(int pos=0; nextPos(path, pos); level++) {
		// array index in path
		if(sscanf(JSONbuffer, "%d", &index)) {
			if(result->type!='a') {
				snprintf(JSONbuffer+24, 79-24, "Type `a` expected on level `%d`, got `%c`.", level, result->type);
				throw JSONbuffer+24;
			}
      result = (JSONList*)result->value;
			while(index-->0) {
				if(result==NULL) {
					snprintf(JSONbuffer+24, 79-24, "Index not found on level `%d`.", level);
					throw JSONbuffer+24;
				}
				result = result->next;
			}
		}
		// group index in path
		else if(JSONbuffer[0]=='[') {
			int i;
			for(i=1; JSONbuffer[i]!=']'; ++i) {
				if(!JSONbuffer[i] || i>23) throw "Missing `]` in parsing syntax.";
			}
			JSONbuffer[i] = 0;
			char* group = JSONbuffer+1;
			result = result->groupPath(group);
		}
		// object key in path
		else {
			if(result->type!='o') {
				snprintf(JSONbuffer+24, 79-24, "Type `o` expected on level `%d`, got `%c`.", level, result->type);
				throw JSONbuffer+24;
			}
      result = (JSONList*)result->value;
			while(strcmp(result->key, JSONbuffer)!=0) {
				if(result==NULL) {
					snprintf(JSONbuffer+24, 79-24, "Key `%s` not found on level `%d`.", JSONbuffer, level);
					throw JSONbuffer+24;
				}
				result = result->next;
			}
		}
	}
	return result;
}

JSONList* JSONList::groupPath(const char* group) {
	if(type!='a') throw "Type `a` expected on group path.";
	JSONList* node = new JSONList();
	JSONList* result = node;
	result->type = 'a';
	int index = -1;
	sscanf(group, "%d", &index);
  for(JSONList* i=(JSONList*)value; i!=NULL; i=i->next) {
		if(i->type!='a' && i->type!='o') continue;
		if(i->type=='a' && index<0) continue;
		int pj = 0;
		for(JSONList* j=(JSONList*)i->value; j!=NULL; j=j->next, pj++) {
			if(i->type=='a' && pj!=index) continue;
			if(i->type=='o' && strcmp(j->key,group)!=0) continue;
			JSONList* item = new JSONList(*j);
			if(node->value==NULL) node->value = item;
			else node->next = item;
			node = item;
		}
	}
	if(select!=NULL) delete select;
	select = result;
	return select;
}

const char* JSONList::nextPos(const char* path, int& pos) {
	if(!path[pos]) return NULL;
	int start = pos;
	while(path[pos] && path[pos]!='/') pos++;
	strncpy(JSONbuffer, path+start, pos-start);
	JSONbuffer[pos-start] = 0;
	if(path[pos]) pos++;
	return JSONbuffer;
}


// JSON

void JSON::parseValue(JSONList* item) {
	// scan value type
	char c;
	sscanf(input+pos, " %c%n", &c, &inc); pos+= inc-1; // unread first non-whitespace char
	switch(c) {
		case '[':	item->type = 'a';	break;
		case '{':	item->type = 'o';	break;
		case '"': item->type = 's'; break;
		default: {
			if(c<'0' || c>'9') throw "Error parsing value.";
			int i;
			for(i=pos; input[i]>='0' && input[i]<='9'; ++i);
			item->type = input[i]=='.' ? 'f' : 'i';
		}
	}

	// scan value
	inc = 0;
	switch(item->type) {
		case 'i': { // integer
			item->value = new int;
			sscanf(input+pos, "%d %n", item->value, &inc); pos+= inc;
			#ifdef DEBUG
			printf("Parsed integer `%d`.\n", item->getInt());
			#endif
		} break;
		case 'f': { // float
			item->value = new float;
			sscanf(input+pos, "%f %n", item->value, &inc); pos+= inc;
			#ifdef DEBUG
			printf("Parsed float `%f`.\n", item->getFloat());
			#endif
		} break;
		case 's': { // string
			int end;
			sscanf(input+pos, "\"%79[^\"]%n %*[^\"]\" %n", JSONbuffer, &end, &inc);
			if(!inc) inc = end+1; pos+= inc;
			item->value = new char[end+1];
			strcpy((char*)item->value, JSONbuffer);
			#ifdef DEBUG
			printf("Parsed string `%s`.\n", item->getString());
			#endif
		} break;
		case 'a': item->value = JSON::parseObject(true); break;
		case 'o': item->value = JSON::parseObject(false); break;
	}
}


JSONList* JSON::parseObject(bool isArray) {
	JSONList* result = NULL;
	JSONList* node = NULL;
	char opening = isArray ? '[' : '{';
	char closing = isArray ? ']' : '}';
	sprintf(JSONbuffer, " %c%%n", opening);
	inc = 0; sscanf(input+pos, JSONbuffer, &inc); pos+= inc;
	if(!inc) {
		sprintf(JSONbuffer, "Missing `%c`.", opening);
		throw JSONbuffer;
	}
	int index = 0;

SCAN_ITEM:
	JSONList* item = new JSONList();

	// scan key
	if(!isArray) {
		if(!sscanf(input+pos, " \"%[^\"]\" : %n", JSONbuffer, &inc)) throw "Missing key.";
		pos+= inc;
		item->key = new char[strlen(JSONbuffer)+1];
		strcpy(item->key, JSONbuffer);
		#ifdef DEBUG
			printf("Parsing object for key `%s`...\n", item->key);
		#endif
	}
	else {
		#ifdef DEBUG
			printf("Parsing array index #%d...\n", index++);
		#endif
	}

	// parse and add value
	parseValue(item);
	if(result==NULL) result = item;
	if(node==NULL) node = item;
	else {
		node->next = item;
		node = item;
	}

	// scan until closing bracket and return
	char c;
	sscanf(input+pos, " %c%n", &c, &inc); pos+= inc;
	if(c==',') goto SCAN_ITEM;
	if(c!=closing) {
		sprintf(JSONbuffer, "Missing `%c`.", closing);
		throw JSONbuffer;
	}
	return result;
}

JSONList JSON::parse(const char* input) {
	JSON::input = input;
	JSONList root;// = new JSONList();
	root.type = 'o';
	root.value = parseObject(false);
	instance = root;
	return root;
}

char* JSON::getState() {
	snprintf(JSONbuffer, 79, "[%d] %s", pos, input+pos);
	return JSONbuffer;
}

int JSON::pos;
int JSON::inc;
const char* JSON::input = "";
JSONList JSON::instance;