#include <stdio.h>
#include <string.h>

int GetStrLen1 (char s[]);
int GetStrLen2 (char s[]);
int GetStrLen3 (char s[]);

int main ()
{
	char arr[] = "Penguins have adapted.";
	int len = strlen (arr);
	printf ("Func: %d StrLen: %d \n", GetStrLen3 (arr), len);

	return 0;
}

int GetStrLen1 (char s[])
{
  	int len = 0;
	for (; s[len] != '\0'; ++len) {}
	return len;
}

int GetStrLen2 (char s[])
{
  	int len = 0;
	while (s[len] != '\0') {++len;}
  	return len;
}

int GetStrLen3 (char s[])
{
	int len = 0;
	while (1) {if (s[len] != '\0') {++len;} else {break;};}
	return len;
}




