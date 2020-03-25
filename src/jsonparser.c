#include "jsonparser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct JSON
{
	int type;

	char* name;
	char* stringval;
	float numval;
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
		JSON_MSG_FUNC("File %s contains none or invalid json data", filepath);
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
			char c = *str;

			// Skip whitespace
			if (c == ' ' || c == '\t' || c == '\n')
				continue;

			// The end of the object
			if (!in_quotes && c == '}')
			{
				return str + 1;
			}

			// Start quote
			if (!in_quotes && c == '"')
			{
				in_quotes = 1;
				opening_quote = str + 1;
				continue;
			}

			// The quote for the name has ended
			// Save the name of the key
			if (in_quotes && c == '"')
			{
				in_quotes = 0;
				if (opening_quote == NULL)
				{
					JSON_MSG_FUNC("Mismatched quote on line %10s", str - 5);
					return NULL;
				}
				size_t lname = str - opening_quote;
				tmp_name = malloc(lname + 1);
				memcpy(tmp_name, opening_quote, lname);
				tmp_name[lname] = '\0';
				continue;
			}

			// Next side of key value pair
			// After reading the key, recursively load the value
			if (!in_quotes && c == ':')
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
					JSON_MSG_FUNC("Invalid json");
				}
				str = tmp_buf;

				// Insert member
				json_add_member(object, tmp_name, new_object);
				free(tmp_name);
				continue;
			}
		}
	}

	// Array
	else if (str[0] == '[')
	{
		object->type = JSON_TARRAY;
		int in_quotes = 0;

		char* opening_quote = NULL;
		char* tmp_name = NULL;
		str++;
		for (; *str != '\0'; str++)
		{
			char c = *str;

			// Skip whitespace
			if (c == ' ' || c == '\t' || c == '\n')
				continue;

			// The end of the array
			if (!in_quotes && c == ']')
			{
				return str + 1;
			}

			// Read elements of array
			if (!in_quotes && c == ':')
			{
				// Load the json with what is after the ':'
				JSON* new_object = malloc(sizeof(JSON));

				// Load the element from the string
				char* tmp_buf = json_load(new_object, str);
				if (tmp_buf == NULL)
				{
					JSON_MSG_FUNC("Invalid json");
				}
				str = tmp_buf;

				// Insert child
				json_add_member(object, tmp_name, new_object);
				free(tmp_name);
				continue;
			}
		}
	}

	// String
	else if (str[0] == '"')
	{
		object->type == JSON_TSTRING;
		// Skip start quote
		str++;
		char* start = str;
		// Loop to end of quote
		for (; *str != '\0'; str++)
		{
			char c = *str;

			if (c == '\t' || c == '\n')
			{
				JSON_MSG_FUNC("Invalid character in string %10s, control characters must be escaped", str - 5);
				return NULL;
			}

			// End quote
			if (c == '"')
			{
				size_t lval = str - start;
				object->stringval = malloc(lval + 1);
				memcpy(object->stringval, start, lval);
				object->stringval[lval] = '\0';

				// Jump to next element
				return str + 1;
			}
		}
	}

	return str;
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
	}
	else
	{
		// Insert at beginning of linked list
		object->members->prev = value;
		value->next = object->members;
		object->members = value;
	}
}

void json_add_element(JSON* object, JSON* value)
{
	JSON* it = object->members;
	// Traverse to end of array
	while (it->next)
	{
		it = it->next;
	}
	it->next = value;
}

void json_insert_element(JSON* object, size_t index, JSON* element)
{
	JSON* it = object->members;
	size_t i = 0;
	// Jump to index or end of array
	while(i < index && it->next)
	{
		it = it->next;
		i++;
	}
	element->prev = it;
	JSON* eit = element;
	while(eit->next)
	{
		eit = eit->next;
	}
	eit->next = it->next;
}

void json_destroy(JSON* object)
{
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

	JSON* it = object->members;
	while (it)
	{
		JSON* next = it->next;
		json_destroy(it);
		it = next;
	}
}