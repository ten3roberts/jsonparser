#include <stddef.h>

typedef struct JSON JSON;

#define JSON_COMPACT 0
#define JSON_FORMAT	 1

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


// Will remove a json object's type and free all previous values and members
// Effectively resets an object while keeping it's parent structure intact
void json_set_invalid(JSON* object);
// Sets the value of the json object to a string
// Previous value is freed
void json_set_string(JSON* object, const char* str);

// Sets the value of the json object to a number
// Previous value is freed
void json_set_number(JSON* object, double num);

// Sets the value of the json object to a bool
// Previous value is freed
void json_set_bool(JSON* object, int val);

// Sets the value of the json object to null
// Previous value is freed
void json_set_null(JSON* object);

// Returns the type of the json object
int json_get_type(JSON* object);

// Returns a pointer to the internal string
// Can be modified
// Validity of pointer is not guaranteed after json_set_string or similar call
// Returns NULL if it's not a string type
char* json_get_string(JSON* object);

// Returns the number value
// Returns 0 if it's not a number type
double json_get_number(JSON* object);

// Returns the bool value
// Returns 0 if it's not a bool type
int json_get_bool(JSON* object);

// Returns a linked list of the members of a json object
JSON* json_get_members(JSON* object);

// Returns the member with the specified name in a json object
JSON* json_get_member(JSON* object, const char* name);

// Returns a linked list of the elements of a json array
JSON* json_get_elements(JSON* object);

// Returns the next item in the list element is a part of
JSON* json_get_next(JSON* element);

// Allocates and returns a json structure as a string
// Returned string needs to be manually freed
// If format is 0, resulting string will not contain whitespace
// If format is 1, resulting string will be pretty formatted
char* json_tostring(JSON* object, int format);

// Writes the json structure to a file
// Not that the name of the root object, if not NULL, is the file that was read
// Returns 0 on success
// Overwrites file
// Creates the directories leading up to it (JSON_USE_POSIX or JSON_USE_WINAPI need to be defined accordingly)
// If format is JSON_COMPACT (1), resulting string will not contain whitespace
// If format is JSON_FORMAT (0), resulting string will be pretty formatted
int json_writefile(JSON* object, const char* filepath, int format);

// Loads a json file recusively from a file into memory
JSON* json_loadfile(const char* filepath);

// Loads a json string recursively
JSON* json_loadstring(char* str);

// Loads a json object from a string
// Returns a pointer to the end of the object in the beginning string
// NOTE : should not be used on an existing object, object needs to be empty or destroyed
char* json_load(JSON* object, char* str);

// Insert a member to a json object with name
// If an object of that name already exists, it is overwritten
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