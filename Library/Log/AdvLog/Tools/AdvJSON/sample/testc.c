#include <stdio.h>
#include <string.h>
#include "AdvJSON.h"


int main(int argc, char **argv) {
	char jsonbuffer[4096] = {0};
	FILE *fp = fopen("example.json","r");
	if(fp == NULL) {
		printf("No File\n");
		return -1;
	}
	fread(jsonbuffer, sizeof(jsonbuffer), 1, fp);
	fclose(fp);
	//printf("jsonbuffer = (%d)\n%s\n", strlen(jsonbuffer),jsonbuffer);
	//int result = JSON_Validator(jsonbuffer);
	//printf("result = %d, len = %d\n", result, (int)strlen(jsonbuffer));
	//JSON_ShowError(jsonbuffer, result);
	
	int result = JSON_Validator(jsonbuffer);
	printf("result = %d, len = %d\n", result, (int)strlen(jsonbuffer));
	char string[4096];
	JSONode *json = JSON_Parser(jsonbuffer);
	char *tag = "[NetID]";
	JSON_Get(json, tag, string, 4096);
	
	printf("tag = %s\nstring = %s\n", tag, string);
	
	
	return 0;

}