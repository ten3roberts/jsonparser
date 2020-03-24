#include "jsonparser.h"
#include <stdio.h>
#include <stdlib.h>

struct JSON
{
	int type;

	char* name;
	char* stringval;
	float numval;
	size_t depth;
	struct JSON* children;
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
	if (json_load(root, buf))
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
	return str;
}
