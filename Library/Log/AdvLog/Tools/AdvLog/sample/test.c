#include <stdio.h>
#include <unistd.h>
#include "AdvLog.h"



int main(int argc, char **argv) {
	AdvLog_Init();
	//AdvLog_Arg(argc,argv);
	char filename[256];
	sprintf(filename,"-f ./%s -n default","log.ini");
	printf("filename = %s\n", filename);
    	AdvLog_Default(filename);

do {
	ADV_C_TRACE(COLOR_RED,"Hello World!!\n");
	ADV_C_DEBUG(COLOR_GREEN,"Hello World!!\n");	
	ADV_C_INFO(COLOR_YELLOW,"Hello World!!\n");
	ADV_C_NOTICE(COLOR_BLUE,"<Hello World!!>\n");
	ADV_C_WARN(COLOR_PURPLE,"Hello World!!\n");	
	ADV_C_ERROR(COLOR_CYAN,"Hello World!!\n");
	ADV_C_CRASH(COLOR_GRAY,"Hello World!!\n");
	printf("\n\n");
	ADV_INFO("\n\n");
	
	
	//printf("\033[31maaa\n\033[0m");
	//printf("\033[0;41mBBB\n\r\033[0m\033[30;42m\033[0m");
	ADV_C_INFO(COLOR_RED_BG,"Hello World!!\n");
	ADV_C_INFO(COLOR_GREEN_BG,"Hello World!!\n");	
	ADV_C_INFO(COLOR_YELLOW_BG,"Hello World!!\n");
	ADV_C_INFO(COLOR_BLUE_BG,"Hello World!!\n");
	ADV_C_INFO(COLOR_PURPLE_BG,"Hello World!!\n");	
	ADV_C_INFO(COLOR_CYAN_BG,"Hello World!!\n");
	ADV_C_INFO(COLOR_WHITE_BG,"Hello World!!\n");
	printf("\n\n");
	ADV_INFO("\n\n");
	sleep(3);
} while(1);
/*
	do {
		ADV_INFO("a = %d, b = %d",100, 200);
		ADV_C_WARN(COLOR_GREEN,"a = %d, b = %d",100, 200);
		sleep(1);
	} while(1);
*/
}




