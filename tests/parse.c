#include "src/jsonparser.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
	JSON* root = json_loadfile("./tests/grid.json");
	if (root == NULL)
	{
		printf("Invalid file\n");
		return -1;
	}
	char* str = json_tostring(root, JSON_FORMAT);
	JSON* root2 = json_loadstring(str);
	json_writefile(root2, "./tests/out/out.json", 1);
	free(str);


	json_destroy(root);
	puts("Done");
}