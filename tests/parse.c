#include "src/jsonparser.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main()
{
	JSON* root = json_loadfile("./tests/example.json");

	printf("Name = %s\n", json_get_string(json_get_member(root, "name")));

	json_writefile(root, "./tests/out/out.json", JSON_FORMAT);

	json_destroy(root);
}