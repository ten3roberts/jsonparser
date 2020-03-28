#include "jsonparser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#if JSON_USE_POSIX
#include <sys/stat.h>
#elif JSON_USE_WINAPI
#include <windows.h>
#else
#error "Please define either 'JSON_USE_POSIX' or 'JSON_USE_WINAPI' depending on platform"
#endif

#define IS_WHITESPACE(c) (c == ' ' || c == '\n' || c == '\r' || c == '\t')

struct StringStream
{
	// The internal string pointer
	char* str;
	// The allocated size of the string
	size_t size;
	// How much has been written to the string
	// Does not include the null terminator
	size_t length;
};

#define WRITE_ESCAPE(c)           \
	escaped_str[escapedi] = '\\';  \
	escaped_str[escapedi + 1] = c; \
	escapedi++;

// Writes to string stream
// If escape is 1, control characters will be escaped as two characters
// Escaping decreases performance
void ss_write(struct StringStream* ss, const char* str, int escape)
{
	if (str == NULL)
	{
		JSON_MSG_FUNC("Error writing invalid string");
	}

	size_t lstr = strlen(str);
	char* escaped_str = NULL;
	if (escape)
	{
		// Worst case, expect all characters to be escaped and fit \0
		escaped_str = malloc(2 * lstr + 1);

		// Allocated size
		// Look for control characters
		size_t stri = 0, escapedi = 0;
		for (; stri < lstr; stri++, escapedi++)
		{
			switch (str[stri])
			{
			case '"':
				WRITE_ESCAPE('"');
				break;
			case '\b':
				WRITE_ESCAPE('b');
				break;
			case '\f':
				WRITE_ESCAPE('f');
				break;
			case '\n':
				WRITE_ESCAPE('n');
				break;
			case '\r':
				WRITE_ESCAPE('r');
				break;
			case '\t':
				WRITE_ESCAPE('t');
				break;
			default:
				escaped_str[escapedi] = str[stri];
				break;
			}
		}
		lstr = escapedi;
		escaped_str[lstr] = '\0';
	}

	// Allocate first time
	if (ss->str == NULL)
	{
		ss->size = 8;
		ss->str = malloc(8);
	}

	// Resize
	if (ss->length + lstr + 1 > ss->size)
	{

		ss->size = ss->size << (size_t)log2(lstr + 1);
		char* tmp = realloc(ss->str, ss->size);
		if (tmp == NULL)
		{
			JSON_MSG_FUNC("Failed to allocate memory for string stream\n");
			return;
		}
		ss->str = tmp;
	}

	// Copy data
	memcpy(ss->str + ss->length, escape ? escaped_str : str, lstr);
	ss->length += lstr;
	// Null terminate
	ss->str[ss->length] = '\0';
	if (escape)
	{
		free(escaped_str);
	}
}

float max(float a, float b)
{
	return (a > b ? a : b);
}

// Converts a double to a string
// Precision indicates the max digits to include after the comma
// Prints up to precision digits after the comma, can write less. Can be used to print integers, where the comma is not
// written Returns how many characters were written
int ftos(double num, char* buf, int precision)
{
	if (isinf(num))
	{
		if (num < 0)
			*buf++ = '-';
		*buf++ = 'i';
		*buf++ = 'n';
		*buf++ = 'f';
		*buf++ = '\0';
		return num < 0 ? 4 : 3;
	}

	// Save the sign and remove it from num
	int neg = num < 0;
	if (neg)
		num *= -1;
	// Shift decimal to precision places to an int
	size_t a = num * pow(10, precision + 1);

	if (a % 10 >= 5)
		a += 10;
	a /= 10;

	int dec_pos = precision;

	// Carried the one, need to round once more
	while (a % 10 == 0 && a && a > num)
	{
		if (a % 10 >= 5)
			a += 10;
		a /= 10;
		dec_pos--;
	}

	int base = 10;
	char numerals[17] = {"0123456789ABCDEF"};

	// Return and write one character if float == 0 to precision accuracy
	if (a == 0)
	{
		*buf++ = '0';
		*buf = '\0';
		return 1;
	}

	size_t buf_index = log10(a) + (dec_pos ? 2 : 1) + max(dec_pos - log10(a), 0) + neg;
	int return_value = buf_index;

	buf[buf_index] = '\0';
	while (buf_index)
	{
		buf[--buf_index] = numerals[a % base];
		if (dec_pos == 1)
			buf[--buf_index] = '.';
		dec_pos--;
		a /= base;
	}
	if (neg)
		*buf = '-';
	return return_value;
}

// Convert a json valid number representation from string to double
char* stof(char* str, double* out)
{
	double result = 0;
	// Signifies if read past period
	int isfraction = 0;
	// The value of the digit after period
	double digitval = 1;
	// Signifies if optional exponent is being read
	int isexponent = 0;
	// Value of exponent
	int exponent = 0;
	// The sign of the resulting value
	int sign = 1;
	if (*str == '-')
		sign = -1;

	for (; *str != '\0'; str++)
	{
		// Decimal place
		if (*str == '.')
		{
			isfraction = 1;
			continue;
		}

		// Exponent
		if (*str == 'e' || *str == 'E')
		{
			if (isexponent)
			{
				JSON_MSG_FUNC("Can't have stacked exponents\n");
				return 0;
			}
			isexponent = 1;
			continue;
		}
		// End
		// Keep comma and return
		if (*str == ',')
		{
			break;
		}

		// Break when not a valid number
		// Triggered when next char is end of object or array wihtout comma
		if (*str < '0' || *str > '9')
		{
			break;
		}

		int digit = *str - '0';
		if (isexponent)
		{
			exponent *= 10;
			exponent += digit;
		}
		else if (isfraction)
		{
			digitval *= 0.1;
			result += digit * digitval;
		}
		// Normal digit
		else
		{
			result *= 10;
			result += digit;
		}
	}
	result *= sign;
	if (exponent != 0)
		*out = result * powf(10, exponent);
	else
		*out = result;
	return str;
}

// Reads from start quote to end quote and takes escape characters into consideration
// Allocates memory for output string; need to be freed manually
char* read_quote(char* str, char** out)
{
	// Skip past start quote
	while (*str != '"')
		str++;
	str++;

	// Iterator for the object stringval
	size_t lval = 8;
	*out = malloc(lval);
	char* result = *out;
	size_t valit = 0;
	// Loop to end of quote
	for (; *str != '\0'; str++)
	{
		char c = *str;

		// Allocate more space for string
		if (valit + 1 >= lval)
		{
			lval *= 2;
			char* tmp = realloc(*out, lval);
			if (tmp == NULL)
			{
				JSON_MSG_FUNC("Failed to allocate memory for string value\n");
				return str;
			}
			*out = tmp;
			result = *out;
		}

		// Escape sequence
		if (c == '\\')
		{
			switch (str[1])
			{
			case '"':
				result[valit++] = '"';
				break;
			case '\\':
				result[valit++] = '\\';
				break;
			case '/':
				result[valit++] = '/';
				break;
			case 'b':
				result[valit++] = '\b';
				break;
			case 'f':
				result[valit++] = '\f';
				break;
			case 'n':
				result[valit++] = '\n';
				break;
			case 'r':
				result[valit++] = '\r';
				break;
			case 't':
				result[valit++] = '\t';
				break;
			default:
				JSON_MSG_FUNC("Invalid escape sequence\n");
				break;
			}
			str++;
			continue;
		}

		if (c == '\\' || c == '\b' || c == '\f' || c == '\n' || c == '\r' || c == '\t')
		{
			JSON_MSG_FUNC("Invalid character in string %10s, control characters must be escaped\n", str);
			return NULL;
		}

		// End quote
		if (c == '"')
		{
			result[valit++] = '\0';
			return str + 1;
		}

		// Normal character
		result[valit++] = c;
	}
	JSON_MSG_FUNC("Unexpected end of string\n");
	return str;
}

struct JSON
{
	int type;

	char* name;
	char* stringval;
	double numval;
	struct JSON* members;

	// Linked list to the other members
	// First elements are more recent
	struct JSON* prev;
	struct JSON* next;
};

JSON* json_create_empty()
{
	JSON* object = calloc(1, sizeof(JSON));
	object->type = JSON_TINVALID;
	return object;
}
JSON* json_create_null()
{
	JSON* object = calloc(1, sizeof(JSON));
	object->type = JSON_TNULL;
	return object;
}

JSON* json_create_string(const char* str)
{
	JSON* object = calloc(1, sizeof(JSON));
	object->type = JSON_TSTRING;
	size_t lstr = strlen(str);
	object->stringval = malloc(lstr + 1);
	memcpy(object->stringval, str, lstr);
	object->stringval[lstr] = '\0';
	return object;
}

JSON* json_create_number(double value)
{
	JSON* object = calloc(1, sizeof(JSON));
	object->type = JSON_TNUMBER;
	object->numval = value;
	return object;
}

JSON* json_create_object()
{
	JSON* object = calloc(1, sizeof(JSON));
	object->type = JSON_TOBJECT;
	return object;
}

JSON* json_create_array()
{
	JSON* object = calloc(1, sizeof(JSON));
	object->type = JSON_TARRAY;
	return object;
}

#define WRITE_NAME                                \
	if (object->name)                             \
	{                                             \
		ss_write(ss, "\"", 0);                    \
		ss_write(ss, object->name, 1);            \
		ss_write(ss, format ? "\": " : "\":", 0); \
	}

void json_tostring_internal(JSON* object, struct StringStream* ss, int format, size_t depth)
{
	if (object->type == JSON_TOBJECT || object->type == JSON_TARRAY)
	{
		WRITE_NAME;
		ss_write(ss, (object->type == JSON_TOBJECT ? "{" : "["), 0);
		if (format)
			ss_write(ss, "\n", 0);

		JSON* it = object->members;
		while (it)
		{
			// Format with tabs
			if (format)
			{
				for (size_t i = 0; i < depth + 1; i++)
				{
					ss_write(ss, "\t", 0);
				}
			}
			json_tostring_internal(it, ss, format, depth + 1);
			it = it->next;
			if (it)
				ss_write(ss, (format ? ",\n" : ","), 0);
		}
		if (format)
		{
			ss_write(ss, "\n", 0);
			for (size_t i = 0; i < depth; i++)
			{
				ss_write(ss, "\t", 0);
			}
		}
		ss_write(ss, (object->type == JSON_TOBJECT ? "}" : "]"), 0);
	}
	else if (object->type == JSON_TSTRING)
	{
		WRITE_NAME;
		ss_write(ss, "\"", 0);

		ss_write(ss, object->stringval, 1);
		ss_write(ss, "\"", 0);
	}
	else if (object->type == JSON_TNUMBER)
	{
		WRITE_NAME;
		char buf[128];
		ftos(object->numval, buf, 5);
		ss_write(ss, buf, 0);
	}
	else if (object->type == JSON_TBOOL)
	{
		WRITE_NAME;
		ss_write(ss, object->numval ? "true" : "false", 0);
	}
	else if (object->type == JSON_TNULL)
	{
		WRITE_NAME;
		ss_write(ss, "null", 0);
	}
}

char* json_tostring(JSON* object, int format)
{
	struct StringStream ss = {0};

	json_tostring_internal(object, &ss, format, 0);
	return ss.str;
}

int json_writefile(JSON* object, const char* filepath, int format)
{
// Create directories leading up
#if JSON_USE_POSIX
	const char* p = filepath;
	char tmp_path[FILENAME_MAX];
	size_t len = 0;
	struct stat st = {0};
	for (; *p != '\0'; p++)
	{
		// Dir separator hit
		if (*p == '/')
		{
			len = p - filepath;

			memcpy(tmp_path, filepath, len);
			tmp_path[len] = '\0';

			// Dir already exists
			if (stat(tmp_path, &st) == 0)
			{
				continue;
			}
			if (mkdir(tmp_path, 0777))
			{
				JSON_MSG_FUNC("Failed to create directory %s\n", tmp_path);
				return -1;
			}
		}
	}
#elif JSON_USE_WINAPI
	char* p = filepath;
	const char tmp_path[FILENAME_MAX];
	for (; *p != '\0'; p++)
	{
		// Dir separator hit
		if (*p == '/' || *p == '\\')
		{
			size_t len = p - filepath;
			memcpy(tmp_path, filepath, p - filepath);
			tmp_path[len] = '\0';
			CreateDirectoryA(tmp_path, NULL);
		}
	}
#endif
	FILE* fp = NULL;
	fp = fopen(filepath, "w");
	if (fp == NULL)
	{
		JSON_MSG_FUNC("Failed to create or open file %s\n", filepath);
		return -2;
	}
	struct StringStream ss = {0};
	json_tostring_internal(object, &ss, format, 0);
	fwrite(ss.str, 1, ss.length, fp);

	// Exit
	free(ss.str);
	fclose(fp);
	return 0;
}

JSON* json_loadfile(const char* filepath)
{
	FILE* fp;
	fp = fopen(filepath, "r");
	if (fp == NULL)
	{
		JSON_MSG_FUNC("Failed to open file %s\n", filepath);
		return NULL;
	}

	// Read the file into a string
	char* buf = NULL;
	size_t size;
	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	buf = malloc(size);
	fseek(fp, 0L, SEEK_SET);
	fread(buf, 1, size, fp);
	fclose(fp);

	JSON* root = malloc(sizeof(JSON));
	size_t lfilepath = strlen(filepath);
	root->name = malloc(lfilepath + 1);
	memcpy(root->name, filepath, lfilepath);
	root->name[lfilepath] = '\0';
	if (json_load(root, buf) == NULL)
	{
		JSON_MSG_FUNC("File %s contains none or invalid json data\n", filepath);
		free(root);
		return NULL;
	}
	return root;
}
JSON* json_loadstring(char* str)
{
	JSON* root = malloc(sizeof(JSON));
	if (json_load(root, str) == NULL)
	{
		JSON_MSG_FUNC("String contains none or invalid json data\n");
		free(root);
		return NULL;
	}
	return root;
}

char* json_load(JSON* object, char* str)
{
	object->name = NULL;
	object->next = NULL;
	object->stringval = NULL;
	object->numval = 0;
	object->type = JSON_TINVALID;
	object->members = NULL;
	object->prev = NULL;
	object->next = NULL;

	// Object
	if (str[0] == '{')
	{
		object->type = JSON_TOBJECT;
		int in_quotes = 0;

		char* tmp_name = NULL;
		str++;
		for (; *str != '\0'; str++)
		{
			// Skip whitespace
			if (IS_WHITESPACE(*str))
			{
				continue;
			}

			// Read the name
			if (*str == '"')
			{
				char* tmp = read_quote(str, &tmp_name);
				if (tmp == NULL)
				{
					JSON_MSG_FUNC("Error reading characters in string \"%.50s\"\n", str);
					break;
				}
				str = tmp;
			}

			// Next side of key value pair
			// After reading the key, recursively load the value
			if (*str == ':')
			{
				// Jump over ':'
				str++;
				// Skip all whitespace after ':'
				while (*str == ' ' || *str == '\t' || *str == '\n')
					str++;

				// Load the json with what is after the ':'
				JSON* new_object = malloc(sizeof(JSON));

				// Load the child element from the string and skip over that string
				char* tmp_buf = json_load(new_object, str);

				if (tmp_buf == NULL || new_object->type == JSON_TINVALID)
				{
					JSON_MSG_FUNC("Invalid json %.15s\n", str);
					json_destroy(new_object);
				}
				else
				{
					str = tmp_buf;
				}

				// Insert member
				json_add_member(object, tmp_name, new_object);
				free(tmp_name);

				// Skip to next comma or quit
				for (; *str != '\0'; str++)
				{
					if (*str == ',')
						break;
					if (*str == '}')
						return str + 1;
					if (IS_WHITESPACE(*str))
						continue;
					JSON_MSG_FUNC("Unexpected character before comma %.15s\n", str);
					return NULL;
				}
				if (*str == '\0')
				{
					JSON_MSG_FUNC("Expected comma before end of string\n");
					return NULL;
				}
				continue;
			}

			JSON_MSG_FUNC("Expected property before \"%.50s\"\n", str);
			return str;
		}
	}

	// Array
	else if (str[0] == '[')
	{
		object->type = JSON_TARRAY;

		str++;
		for (; *str != '\0'; str++)
		{
			// Skip whitespace
			if (IS_WHITESPACE(*str))
				continue;

			// The end of the array
			if (*str == ']')
				return str + 1;

			// Read elements of array

			{
				JSON* new_object = malloc(sizeof(JSON));

				// Load the element from the string
				char* tmp_buf = json_load(new_object, str);
				if (tmp_buf == NULL)
				{
					JSON_MSG_FUNC("Invalid json\n");
					json_destroy(new_object);
				}
				else
				{
					str = tmp_buf;
				}

				// Insert element at end
				json_add_element(object, new_object);

				// Skip to next comma or quit
				for (; *str != '\0'; str++)
				{
					if (*str == ',')
						break;
					if (*str == ']')
						return str + 1;
					if (IS_WHITESPACE(*str))
						continue;
					JSON_MSG_FUNC("Unexpected character before comma \"%.50s\"\n");
					return str;
				}
			}

			// The end of the array
			if (*str == ']')
			{
				return str + 1;
			}
		}
	}

	// String
	else if (str[0] == '"')
	{
		object->type = JSON_TSTRING;
		return read_quote(str, &object->stringval);
	}
	// Number
	else if ((*str >= '0' && *str <= '9') || *str == '-' || *str == '+')
	{
		object->type = JSON_TNUMBER;
		return stof(str, &object->numval);
	}

	// Bool true
	else if (strncmp(str, "true", 4) == 0)
	{
		object->type = JSON_TBOOL;
		object->numval = 1;
		return str + 4;
	}

	// Bool true
	else if (strncmp(str, "false", 5) == 0)
	{
		object->type = JSON_TBOOL;
		object->numval = 0;
		return str + 5;
	}

	else if (strncmp(str, "null", 4) == 0)
	{
		object->type = JSON_TNULL;
		return str + 4;
	}

	return NULL;
}

void json_add_member(JSON* object, const char* name, JSON* value)
{
	size_t lname = strlen(name);
	value->name = malloc(lname + 1);
	memcpy(value->name, name, lname);
	value->name[lname] = '\0';

	if (object->members == NULL)
	{
		object->members = value;
		return;
	}

	// Traverse to end of array and look for duplicate
	JSON* it = object->members;
	while (it->next)
	{
		// Replace duplicate
		if (strcmp(value->name, it->name) == 0)
		{
			value->next = it->next;
			value->prev = it->prev;
			it->next->prev = value;
			if (it->prev)
				it->prev->next = value;
			return;
		}
		it = it->next;
	}
	it->next = value;
	value->prev = it;
}

void json_add_element(JSON* object, JSON* element)
{
	if (object->members == NULL)
	{
		object->members = element;
		return;
	}

	JSON* it = object->members;
	// Traverse to end of array
	while (it->next)
	{
		it = it->next;
	}
	it->next = element;
	element->prev = it;
}

void json_insert_element(JSON* object, size_t index, JSON* element)
{
	JSON* it = object->members;
	size_t i = 0;
	// Jump to index or end of array
	while (i < index && it->next)
	{
		it = it->next;
		i++;
	}
	element->prev = it;
	JSON* eit = element;
	while (eit->next)
	{
		eit = eit->next;
	}
	eit->next = it->next;
}

void json_destroy(JSON* object)
{
	JSON* it = object->members;
	while (it)
	{
		JSON* next = it->next;
		json_destroy(it);
		it = next;
	}

	if (object->name)
	{
		free(object->name);
		object->name = NULL;
	}
	if (object->stringval)
	{
		free(object->stringval);
		object->stringval = NULL;
	}

	object->numval = 0;
	object->type = JSON_TINVALID;

	free(object);
}