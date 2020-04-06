#define JSON_IMPLEMENTATION
#include "libjson.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main()
{
	JSON* root = json_loadfile("./tests/example.json");

	json_destroy(json_remove_member(root, "age"));
	printf("Name = %s\n", json_get_string(json_get_member(root, "name")));
	json_destroy(json_remove_member(root, "name"));

	json_writefile(root, "./tests/out/out.json", JSON_FORMAT);

	json_destroy(root);
}