#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h> 
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <string>
#include <algorithm>
#include "fplatform.h"
#include "tool.h"
#include "configure.h"
#include "rollingfile.h"
#include "tinydir.h"


//size_t strftime(char *s, size_t max, const char *format, const struct tm *tm);
static char pathname[256] = "logs";
static char lastfile[256];
static char filename[256];
static char fullname[256];
static char name[64] = "";
static FILE *logfile = NULL;
static int limit = 102400;
static int currentcount;
static int files = 100; // 100KB * 100 ~= 10 MB
static int staticGray = 0;

bool isDirExist(const std::string& path)
{
#if defined(_WIN32)
    struct _stat info;
    if (_stat(path.c_str(), &info) != 0)
    {
        return false;
    }
    return (info.st_mode & _S_IFDIR) != 0;
#else 
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
    {
        return false;
    }
    return (info.st_mode & S_IFDIR) != 0;
#endif
}

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

bool makePath(const std::string& target, int mode)
{
	std::string path = ReplaceAll(target, "\\", "/");
    int ret = mkdir(path.c_str(), mode);
    if (ret == 0)
        return true;

    switch (errno)
    {
    case ENOENT:
        // parent didn't exist, try to create it
        {
            int pos = path.find_last_of('/');
            if (pos == std::string::npos)
                return false;
            if (!makePath( path.substr(0, pos) , mode))
                return false;
        }
        // now, try to create again

        return 0 == mkdir(path.c_str(), mode);

    case EEXIST:
        // done!
        return isDirExist(path);

    default:
        return false;
    }
}

inline static void RollFile_New() {
	int pid = AdvLog_Configure_GetPid();
	char timestr[256]= {0};
	char checkfilename[256] = {0};
	time_t oldt = 0;
	time_t t = time(NULL);
	struct tm timetm;
	localtime_r(&t, &timetm);
	strftime(timestr, sizeof(timestr), "%Y-%m-%d_%H-%M-%S", &timetm);
	makePath(pathname,0777);

	staticGray = AdvLog_Configure_GetStaticGray();

	do {
		if(oldt == t) sleep(1);
		t = time(NULL);
		localtime_r(&t, &timetm);
		strftime(timestr, sizeof(timestr), "%Y-%m-%d_%H-%M-%S", &timetm);
		snprintf(filename, sizeof(filename), "%s@%d", timestr, pid < 0 ? 0 : pid);
		if(staticGray == 0) {
			snprintf(fullname, sizeof(fullname), "%s/%s%s.html", pathname, filename,name);
		} else {
			snprintf(fullname, sizeof(fullname), "%s/%s%s.log", pathname, filename,name);
	}
		oldt = t;
	} while(access(fullname,F_OK) == 0);
	
	logfile = fopen(fullname,"w");

	if(logfile != NULL) chmod(fullname, 0777);

	if(staticGray == 0 && logfile != NULL) { 
		fprintf(logfile,"<!DOCTYPE html><html><head>" ADV_HTML_CSS_STYLE "</head><body><pre>");
	}
}

void RollFile_Open(const char *path) {
	strncpy(pathname,path,sizeof(pathname));
}

void RollFile_SetLimit(int byte) {
	limit = byte;
}

void RollFile_RefreshConfigure() {
	limit = AdvLog_Configure_GetLimit();
	files = AdvLog_Configure_GetFiles();
	strncpy(pathname,AdvLog_Configure_GetPath(),sizeof(pathname));
	RemoveOverFiles();
}

inline static void RollFile_Rolling() {
	
	if(strlen(lastfile) != 0) { 
		unlink(lastfile);
	}
	if(staticGray == 0 && logfile != NULL) { 
		fprintf(logfile,"</pre></body></html>");
	}
	
	if(logfile != NULL) {
		fclose(logfile);
		logfile = NULL;
	}
	
	if(staticGray == 0) {
		sprintf(lastfile,"%s/__%s%s",pathname,filename,".html");
	} else {
		sprintf(lastfile,"%s/__%s%s",pathname,filename,".log");
	}
	rename(fullname,lastfile);
	
	RollFile_RefreshConfigure();
	
	RollFile_New();
}

FILE *RollFile_Check() {
	if(logfile == NULL) {
		RollFile_New();
	}
	
	if(staticGray != AdvLog_Configure_GetStaticGray()) {
		RollFile_Rolling();
		currentcount = 0;
	}
	return logfile;
}
void RollFile_StreamIn(const char *stream, int length) {
	if(logfile == NULL) {
		RollFile_New();
	}
	
	if(staticGray != AdvLog_Configure_GetStaticGray()) {
		RollFile_Rolling();
		currentcount = 0;
	}
	
	if(logfile != NULL) {
		currentcount += length;
		fwrite(stream, length, 1, logfile);
		fflush(logfile);
	}
	
	if(currentcount >= limit) {
		RollFile_Rolling();
		currentcount = 0;
	}
}

char * GetFolderPath()
{
	char *sp = pathname;
	char workingdir[512]={0};
	static char temp[512]={0};
	char *p = strrchr(pathname,'/');
	if( p != NULL && ( p - sp ) > 3 )
		return pathname;
	else {
			getcwd(workingdir,512);
#if defined(_WIN32)
			snprintf(temp,sizeof(temp),"%s\\%s",workingdir,p+1);
#else
		   snprintf(temp,sizeof(temp),"%s/%s",workingdir,p+1);
#endif
			return temp;
	}
}


void  RemoveOverFiles()
{
	tinydir_dir dir;
	int count = 0;
	int i = 0, j = 0;
	int rmfs = 0;
	char allfiles[210][256];
	char temp[256]={0};
	sprintf(temp,"%s",GetFolderPath());
	tinydir_open(&dir, temp );

	sprintf(name, "__%s", AdvLog_Configure_Name() );

	if( !strncmp( name, "__default", 9 ) || strlen(name) == 2 )
		memset(name, 0, sizeof(name));
	
	i = strlen(name);

	while (dir.has_next)
	{
		tinydir_file file;
		tinydir_readfile(&dir, &file);
		
		if (file.is_dir) {
		//	printf("/");
		} else {			
			if( count < 200 )  // check my file name
			{
				// only remove specific application's files
				if( ( strstr( file.name, name ) && strstr( name, "__" ) )  || ( i == 0 && !strstr(file.name, "__") ) )
				{
					memset(allfiles[count],0,256);
					snprintf(allfiles[count],256,"%s",file.path);
					++count;
				}
			}
		}

		tinydir_next(&dir);
	}

	tinydir_close(&dir);

	for(i = 0;i < count;i++) 
	{
		for(j = i+1; j < count; j++)
		{           
			if(strcmp(allfiles[i],allfiles[j]) > 0)
			{            
				memset(temp,0,256);
				strcpy(temp,allfiles[i]);      
				strcpy(allfiles[i],allfiles[j]);      
				strcpy(allfiles[j],temp);    
			}    
		} 
	}


	printf("file count total=%d max:%d\n", count, files);
	if( count > files )
		rmfs = count - files;

	for( i = 0; i < rmfs ; i ++ )
		remove( allfiles[i] );

}

void RollFile_Flush(int length) {
	if(logfile != NULL) {
		currentcount += length;
		fflush(logfile);
	}
	if(currentcount >= limit) {
		RollFile_Rolling();
		currentcount = 0;
	}
}

void RollFile_Close() {
	if(logfile != NULL) {
		fclose(logfile);
		logfile = NULL;
	}
}
