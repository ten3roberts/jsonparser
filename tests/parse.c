#include "src/jsonparser.h"
#include <stdio.h>
#include <stdlib.h>

char* names[] = {"Emma",   "Olivia", "Ava",		"Isabella", "Sophia",	 "Charlotte", "Mia",	"Amelia",
				 "Harper", "Evelyn", "Abigail", "Emily",	"Elizabeth", "Mila",	  "Ella",	"Avery",
				 "Sofia",  "Camila", "Liam",	"Noah",		"William",	 "James",	  "Oliver", "Benjamin",
				 "Elijah", "Lucas",	 "Mason",	"Logan",	"Alexander", "Ethan",	  "Jacob",	"Michael",
				 "Daniel", "Henry",	 "Jackson", "Sebastian"};

JSON* person_create(size_t depth)
{
	JSON* person = json_create_object();
	json_add_member(person, "name", json_create_string(names[rand() % sizeof(names) / sizeof(*names)]));
	json_add_member(person, "age", json_create_number(rand() % 10 + 10));
	json_add_member(person, "balance", json_create_number(rand() / (double)RAND_MAX * 1000));
	if (depth > 0)
	{
		JSON* friends = json_create_array();
		json_add_member(person, "friends", friends);

		for (size_t i = 0; i <10 ; i++)
			json_add_element(friends, person_create(depth - 1));
	}
	return person;
}

int main()
{
	JSON* root = json_create_empty();
	json_add_member(root, "name", json_create_string("Julia"));
	json_add_member(root, "age", json_create_number(19));
	JSON* friends = json_create_array();
	json_add_member(root, "friends", friends);

	for (size_t i = 0; i < 10; i++)
		json_add_element(friends, person_create(1));

	json_writefile(root, "./tests/out/out.json", JSON_FORMAT);

	json_destroy(root);
	puts("Done");
}