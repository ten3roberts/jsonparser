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


	json_destroy(root);
}