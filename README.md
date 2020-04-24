# Libjson

Libjson is a small and easy to use library for parsing, reading, and generating json

## What it does

## Integrating and building
To use libjson in your program copy the header into your project and include it whilst defining LIBJSON_IMPLEMENTATION in one C file to build it

Example:
```
#include <stdio.h>
#define LIBJSON_IMPLEMENTATION
#include "libjson.h"

...
```
Libjson can be included several times, but only one C file can define LIBJSON_IMPLEMENTATION

## Configuration
Configuring of the library is done at build time by defining zero or more of below macros before the header include in the same file as MP_IMPLEMENTATION

* JSON_MALLOC, JSON_REALLOC, and JSON_FREE to use your own allocators instead of the standard library
* JSON_MESSAGE (default fputs(m, stderr)) to set your own message callback.

## Types
The library represents all json types with the JSON structure
The seven types are
* JSON_TINVALID
* JSON_TOBJECT
* JSON_TARRAY
* JSON_TSTRING
* JSON_TNUMBER
* JSON_TBOOL
* JSON_TNULL
The type can be checked by comparing the return value of json_get_type.

### Objects and Arrays
Objects are an unordered collection of name value pairs

Arrays are an ordered collection of values

In libjson they are both represented as children of a JSON structure with type JSON_TOBJECT or JSON_TARRAY
Each child is a JSON struct on it's own and contains the name and the value of the pair. This means that the members can be looked at independently from the parent object since they store their own name

The children are stored in a double linked list. The difference between object members and array elements is that the name is NULL

To retrieve a member of a certain name, use json_get_member(object), this iterates the linked list until a match is found, and returns NULL if no match is found at end

If you want to loop through the members or elements of a object or array, use json_get_members, or json_get_elements respectively

This will return the first JSON struct in the linked list, the next item can be retrieved with json_get_next(member)

Example:
```
JSON* root = json_loadfile("example.json");
JSON* it = json_get_members(root);
while(it)
{
  if(strcmp(json_get_name(it), "name")
  {
    printf("name is %s\n", json_get_string(it));
  }
  it = json_get_next();
}
json_destroy(root);
```

### Strings
A JSON struct with type string contains a copy of the assigned zero terminated string

The value of the string can be retrieved with json_get_string, this will return the pointer to the internal string and should no be freed, return NULL if not JSON_TSTRING.
The validity of the pointer is not guaranteed after json_set_string or similar call. You can write to the string but not realloc it. Long term storage of the return value is not recommended

### Numbers and Bools
Numbers represent a double precision floating point value. Bools are also a type of number with either the value 1 or 0

### Null
Null is a valid json type and has no other use than indicate the absence of a value

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details

The license is also included in the header libjson.h for single file implementation
