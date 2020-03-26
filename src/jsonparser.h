#include <stddef.h>

typedef struct JSON JSON;

#define JSON_TINVALID 0
#define JSON_TNULL	  1
#define JSON_TSTRING  2
#define JSON_TNUMBER  4
#define JSON_TBOOL	  8
#define JSON_TOBJECT  16
#define JSON_TARRAY	  32

// Defines the function the json parser will use to print error messages
// Usage should be like printf and support %s flag
#define JSON_MSG_FUNC printf

JSON* json_create_empty();
JSON* json_create_null();
JSON* json_create_string(const char* string);
JSON* json_create_number(double value);
JSON* json_create_object();
JSON* json_create_array();

// Loads a json file recusively from a file
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