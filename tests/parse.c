#include "src/jsonparser.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
	JSON* root = json_loadfile("./tests/example.json");
	if (root == NULL)
	{
		printf("Invalid file\n");
		return -1;
	}
	char* str = json_tostring(root);
	puts(str);
	JSON* root2 = json_create_empty();
	(void)json_load(root2, str);
	free(str);


	json_destroy(root);
	puts("Done");
	while(getchar() != '\n');
}