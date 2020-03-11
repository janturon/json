#pragma once

struct JSONList {
	char type; // s/i/f/a/o/n
	char* key; // only for o
	void* value;
	JSONList* next; // for a/o members

	unsigned char* refs;
	JSONList();
	JSONList(const JSONList& copy);
	JSONList& operator=(const JSONList& rhs);

	virtual ~JSONList();
	void valueRefInc();

	int getInt() const;
	float getFloat() const;
	char* getString() const;
	void printValue(bool all=false) const;

	const char* parsedPath;
	JSONList* xpath(const char* path);
	void iterate(void(*f)(void*,char));
	void iterate(void(*f)(const char*));
	void iterate(void(*f)(char*&));
	void iterate(void(*f)(int*));
	void iterate(void(*f)(float*));

private:
	JSONList* select;
	JSONList* groupPath(const char* group);
	const char* nextPos(const char* path, int& pos);
};

class JSON {
protected:
	static int pos, inc;
	static const char* input;
	static JSONList instance;

	static void parseValue(JSONList* item);
	static JSONList* parseObject(bool isArray);

public:
	static JSONList parse(const char* input);
	static char* getState();
};
