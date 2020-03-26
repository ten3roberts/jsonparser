#include "jsonparser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define IS_WHITESPACE(c) (c == ' ' || c == '\n' || c == '\r' || c == '\t')

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
	char* start = str;

	// Iterator for the object stringval
	size_t lval = 2;
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

		if (IS_WHITESPACE(c))
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
	size_t depth;
	struct JSON* members;

	// Linked list to the other members
	// First elements are more recent
	struct JSON* prev;
	struct JSON* next;
};

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
	root->depth = 0;
	if (json_load(root, buf) == NULL)
	{
		JSON_MSG_FUNC("File %s contains none or invalid json data\n", filepath);
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

		char* opening_quote = NULL;
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
					JSON_MSG_FUNC("Error reading characters in string\n");
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
				if (tmp_buf == NULL)
				{
					JSON_MSG_FUNC("Invalid json\n");
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
					JSON_MSG_FUNC("Unexpected character before comma\n");
					return str;
				}
			}

			// The end of the object
			if (!in_quotes && *str == '}')
			{
				return str + 1;
			}
		}
	}

	// Array
	else if (str[0] == '[')
	{
		object->type = JSON_TARRAY;
		int in_quotes = 0;

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
					JSON_MSG_FUNC("Unexpected character before comma\n");
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
	else if ((*str > '0' && *str < '9') || *str == '-' || *str == '+')
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
	value->depth = object->depth + 1;

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
	element->depth = object->depth + 1;

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
		eit->depth = object->depth + 1;
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