#define LIBJSON_IMPLEMENTATION
#define MP_IMPLEMENTATION
#define MP_CHECK_FULL
#include "magpie.h"
#include "libjson.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


int main()
{
	JSON* root = json_loadfile("./tests/example.json");

	json_destroy(json_pop_member(root, "age"));
	json_add_member(root, "name", json_create_string("Jacob"));
	printf("Name = %s\n", json_get_string(json_get_member(root, "name")));
	json_destroy(json_pop_member(root, "name"));

	json_writefile(root, "./tests/out/out.json", JSON_FORMAT);

	json_destroy(root);
	mp_terminate();
}