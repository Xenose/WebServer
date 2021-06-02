#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include"Utils.h"


/* A helper function to create strings */
char* create_string(const char* text)
{
   char* tmp = (char*)malloc(sizeof(char) * strlen(text));
   strcpy(tmp, text);
   return tmp;
}

