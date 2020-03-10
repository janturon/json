# JSON

simple JSON parser.

- To load JSON string into JSONList, use static `JSONList JSON::parse(const char*)`
- To extract desired data of JSONList, use `JSONList* JSONList::xpath(const char*)`
- To print JSONList data, use `JSONList::printValue(true)` (true=print subtrees, false=print value)
- To iterate values, use `JSONList::iterate` method, see `example2.cpp`

## Files

- `json.h` and `json.cpp` - you only need these two files
- `test.hpp` - testing data used in following examples
- `example1.cpp` - print values from JSON subtree
- `example2.cpp` - use callback functions to process JSON values
- `example3.cpp` - low level iteration over JSON values

## JSON values

JSON is implemented as linked list with value of `void*` type. The polymorfic type is stored in `type` attribute, wchich can be:

- 'n' = NULL, undefined value
- 'o' = value is JSON object which can be further iterated
- 'a' = value is JSON array which can be further iterated
- 's' = C string
- 'i' = integer
- 'f' = float
