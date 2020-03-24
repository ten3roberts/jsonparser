#include "src/jsonparser.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
	JSON* root = json_loadfile("grid.json");
	if(root == NULL)
	{
		printf("Invalid file");
		return -1;
	}
}