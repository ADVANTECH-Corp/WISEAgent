#include "util_process.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>

bool util_process_launch(char * appPath)
{
	bool bRet = true;
    pid_t pid = fork();
	if ( 0 == pid ) {/* Child process */
		exit(execlp("/bin/sh", "sh", "-c", appPath, NULL));
	} else if (pid < 0){ /* fork() failed */
        bRet = false;    
	}
	return bRet;
}

bool util_process_kill(char * processName)
{
	pid_t p;
	size_t i, j;
	char* s = (char*)malloc(264);
	char buf[128];
	FILE* st;
	DIR* d = opendir("/proc");
	if (d == NULL) { free(s); return false; }
	struct dirent* f;
	while ((f = readdir(d)) != NULL) {
		if (f->d_name[0] == '.') continue;
		for (i = 0; isdigit(f->d_name[i]); i++);
		if (i < strlen(f->d_name)) continue;
		strcpy(s, "/proc/");
		strcat(s, f->d_name);
		strcat(s, "/status");
		st = fopen(s, "r");
		if (st == NULL) { closedir(d); free(s); return false; }
		do {
			if (fgets(buf, 128, st) == NULL) { fclose(st); closedir(d); free(s); return -1; }
		} while (strncmp(buf, "Name:", 5));
		fclose(st);
		for (j = 5; isspace(buf[j]); j++);
		*strchr(buf, '\n') = 0;
		if (!strcmp(&(buf[j]), processName)) {
			sscanf(&(s[6]), "%d", &p);
			kill(p, SIGKILL);
		}
	}
	closedir(d);
	free(s);
	return true;
}