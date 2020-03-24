typedef struct JSON JSON;

#define JSON_TINVALID 0
#define JSON_TNULL	  1
#define JSON_TSTRING  2
#define JSON_TNUM	  4
#define JSON_TOBJECT  8
#define JSON_TARRAY	  16

// Defines the function the json parser will use to print error messages
// Usage should be like printf and support %s flag
#define JSON_MSG_FUNC printf

// Loads a json file recusively from a file
JSON* json_loadfile(const char* filepath);

// Loads a json object from a string
// Returns a pointer to the end of the object in the beginning string
char* json_load(JSON* object, char* str);
