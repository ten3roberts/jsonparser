#include <stddef.h>

typedef struct JSON JSON;

#define JSON_TINVALID 0
#define JSON_TOBJECT  1
#define JSON_TARRAY	  2
#define JSON_TSTRING  4
#define JSON_TNUMBER  8
#define JSON_TBOOL	  16
#define JSON_TNULL	  32

// Defines the function the json parser will use to print error messages
// Usage should be like printf and support %s flag
#define JSON_MSG_FUNC printf

// Creates an empty json with invalid type
JSON* json_create_empty();
// Creates a valid json null
JSON* json_create_null();

// Creates a json string
// string is copied internally and can be freed afterwards
JSON* json_create_string(const char* str);

// Creates a json number
JSON* json_create_number(double value);

// Creates an empty json object with no members
JSON* json_create_object();

// Creates an empty json array with no elements
JSON* json_create_array();

// Allocates and returns a json structure as a string
// Returned string needs to be manually freed
// String contains tabs or linefeeds
char* json_tostring(JSON* object);

// Loads a json file recusively from a file into memory
JSON* json_loadfile(const char* filepath);

// Loads a json object from a string
// Returns a pointer to the end of the object in the beginning string
// NOTE : should not be used on an existing object, object needs to be empty or destroyed
char* json_load(JSON* object, char* str);

// Insert a member to a json object with name
void json_add_member(JSON* object, const char* name, JSON* value);

// Insert an element to the end of a json array
void json_add_element(JSON* object, JSON* element);

// Insert an element into arbitrary position in a json array
// If index is greater than the length of the array, element will be inserted at the end
// if element is a linked list, the whole list will be inserted in order
void json_insert_element(JSON* object, size_t index, JSON* element);

// Recursively destroys and frees an object
// Will NOT destroy subsequent objects in its array
void json_destroy(JSON* object);