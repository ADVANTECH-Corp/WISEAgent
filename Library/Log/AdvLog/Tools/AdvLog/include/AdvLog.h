/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2015/07/07 by Fred Chang									*/
/* Modified Date: 2015/07/07 by Fred Chang									*/
/* Abstract     : Advantech Logging Library    						        */
/* Reference    : None														*/
/****************************************************************************/
#ifndef __ADV_LOG_H__
#define __ADV_LOG_H__

#if defined(WIN32)
	#pragma once
	#ifdef ADVLOG_EXPORTS
		#define ADVLOG_CALL __stdcall
		#define ADVLOG_EXPORT __declspec(dllexport)
	#else
		#define ADVLOG_CALL __stdcall
		#define ADVLOG_EXPORT
	#endif
	#define __func__ __FUNCTION__
#else
	#define ADVLOG_CALL
	#define ADVLOG_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(NO_DEBUG) 

#define ADV_PRINT(level, fmt, ...)
#define ADV_C_PRINT(level, color, fmt, ...)

#define ADV_CRASH(fmt,...)
#define ADV_C_CRASH(color,fmt,...)

#define ADV_ERROR(fmt,...)
#define ADV_C_ERROR(color,fmt,...)

#define ADV_WARN(fmt,...)
#define ADV_C_WARN(color,fmt,...)

#define ADV_NOTICE(fmt,...)
#define ADV_C_NOTICE(color,fmt,...)

#define ADV_INFO(fmt,...)
#define ADV_C_INFO(color,fmt,...)

/******************************************/

#define ADV_DEBUG(fmt,...)
#define ADV_C_DEBUG(color,fmt,...)

#define ADV_TRACE(fmt,...)
#define ADV_C_TRACE(color,fmt,...)

#define ADV_DUMP(fmt,...)
#define ADV_C_DUMP(color,fmt,...)

#elif defined(NATIVE_PRINTF)

#define ADV_PRINT(level, fmt, ...)				printf(fmt,##__VA_ARGS__)
#define ADV_C_PRINT(level, color, fmt, ...)		printf(fmt,##__VA_ARGS__)

#define ADV_CRASH(fmt,...)  					printf(fmt,##__VA_ARGS__)
#define ADV_C_CRASH(color,fmt,...)  			printf(fmt,##__VA_ARGS__)

#define ADV_ERROR(fmt,...)  					printf(fmt,##__VA_ARGS__)
#define ADV_C_ERROR(color,fmt,...)  			printf(fmt,##__VA_ARGS__)

#define ADV_WARN(fmt,...)  						printf(fmt,##__VA_ARGS__)
#define ADV_C_WARN(color,fmt,...)  				printf(fmt,##__VA_ARGS__)

#define ADV_NOTICE(fmt,...)  					printf(fmt,##__VA_ARGS__)
#define ADV_C_NOTICE(color,fmt,...)				printf(fmt,##__VA_ARGS__)

#define ADV_INFO(fmt,...)  						printf(fmt,##__VA_ARGS__)
#define ADV_C_INFO(color,fmt,...)				printf(fmt,##__VA_ARGS__)

/******************************************/

#define ADV_DEBUG(fmt,...)  					printf(fmt,##__VA_ARGS__)
#define ADV_C_DEBUG(color,fmt,...)  			printf(fmt,##__VA_ARGS__)

#define ADV_TRACE(fmt,...)  					printf(fmt,##__VA_ARGS__)
#define ADV_C_TRACE(color,fmt,...)  			printf(fmt,##__VA_ARGS__)

#define ADV_DUMP(fmt,...)  						printf(fmt,##__VA_ARGS__)
#define ADV_C_DUMP(color,fmt,...)  				printf(fmt,##__VA_ARGS__)

#else


#define S(x) #x
#define S_(x) S(x)
#define S__LINE__ S_(__LINE__)


/**************************************************************************************/

#define __LOG_HEAD_HTML			"<log>"
#define __NODECODE_HTML			"<script type=\"text/plain\">"
#define __BOLD_HTML 			"<b>"
#define __COLOR_RED_HTML 		"<red>"
#define __COLOR_GREEN_HTML 		"<green>"
#define __COLOR_YELLOW_HTML 	"<yellow>"
#define __COLOR_BLUE_HTML 		"<blue>"
#define __COLOR_PURPLE_HTML 	"<purple>"
#define __COLOR_CYAN_HTML 		"<cyan>"
#define __COLOR_GRAY_HTML 		"<gray>"

#define __COLOR_RED_BG_HTML 	"<redbg>"
#define __COLOR_GREEN_BG_HTML 	"<greenbg>"
#define __COLOR_YELLOW_BG_HTML 	"<yellowbg>"
#define __COLOR_BLUE_BG_HTML 	"<bluebg>"
#define __COLOR_PURPLE_BG_HTML 	"<purplebg>"
#define __COLOR_CYAN_BG_HTML 	"<cyanbg>"
#define __COLOR_WHITE_BG_HTML 	"<whitebg>"

#define __COLOR_NONE_HTML 	  	NULL

#define __LOG_END_HTML				"</log>"
#define __NODECODE_END_HTML			"</script>"
#define __BOLD_END_HTML 			"</b>"
#define __COLOR_RED_END_HTML 	  	"</red>"
#define __COLOR_GREEN_END_HTML 	  	"</green>"
#define __COLOR_YELLOW_END_HTML 	"</yellow>"
#define __COLOR_BLUE_END_HTML 	  	"</blue>"
#define __COLOR_PURPLE_END_HTML 	"</purple>"
#define __COLOR_CYAN_END_HTML 	  	"</cyan>"
#define __COLOR_GRAY_END_HTML 	  	"</gray>"

#define __COLOR_RED_BG_END_HTML 	"</redbg>"
#define __COLOR_GREEN_BG_END_HTML 	"</greenbg>"
#define __COLOR_YELLOW_BG_END_HTML 	"</yellowbg>"
#define __COLOR_BLUE_BG_END_HTML 	"</bluebg>"
#define __COLOR_PURPLE_BG_END_HTML 	"</purplebg>"
#define __COLOR_CYAN_BG_END_HTML 	"</cyanbg>"
#define __COLOR_WHITE_BG_END_HTML 	"</whitebg>"

#define __COLOR_NONE_END_HTML 	  	NULL

/**************************************************************************************/

#define LOG_NONE	0
#define LOG_CRASH 	1
#define LOG_ERROR 	2
#define LOG_WARN 	3
#define LOG_NOTICE 	4
#define LOG_INFO 	5
//6
#define LOG_DEBUG   7
#define LOG_TRACE 	8
#define LOG_DUMP 	9

#define LOG_MAX 	10


/*A*/ADVLOG_EXPORT int ADVLOG_CALL AdvLog_AllNone(char *ConfName, int level);
/*B*/ADVLOG_EXPORT int ADVLOG_CALL AdvLog_Static_Is_On(char *ConfName, int level);
/*C*/ADVLOG_EXPORT int ADVLOG_CALL AdvLog_Dynamic_Is_On(char *ConfName, int level);
/*D*/ADVLOG_EXPORT int ADVLOG_CALL AdvLog_Dynamic_Is_Hiden(char *ConfName, int level);

/*E*/ADVLOG_EXPORT void ADVLOG_CALL AdvLog_Print(char *ConfName, int level, int color, const char *file, const char *func, const char *line, const char* levels, const char *fmt, ...);
/*F*/ADVLOG_EXPORT void ADVLOG_CALL AdvLog_Write(char *ConfName, int level, const char *color, const char *colorend, const char *file, const char *func, const char *line, const char* levels, const char *fmt, ...);
/*G*/ADVLOG_EXPORT void ADVLOG_CALL AdvLog_PrintAndWrite(char *ConfName, int level, int color, const char *colorstr, const char *colorend, const char *file, const char *func, const char *line, const char* levels, const char *fmt, ...);
#define __ADV_PRINT(level, color, colorstr, colorend, fmt,...) 	do {\
												if (AdvLog_AllNone("default", level)) {\
													break;\
												} else if (AdvLog_Dynamic_Is_On("default", level) && AdvLog_Static_Is_On("default", level) && !AdvLog_Dynamic_Is_Hiden("default", level)) {\
													AdvLog_PrintAndWrite("default", level, color, colorstr, colorend, __FILE__, __func__, S__LINE__,#level, fmt, ##__VA_ARGS__);\
												} else if(AdvLog_Dynamic_Is_On("default", level) && !AdvLog_Static_Is_On("default", level) && !AdvLog_Dynamic_Is_Hiden("default", level)) {\
													AdvLog_Print("default", level, color,__FILE__, __func__, S__LINE__,#level, fmt, ##__VA_ARGS__);\
												} else if(AdvLog_Static_Is_On("default", level)) {\
													AdvLog_Write("default", level, colorstr,colorend,__FILE__, __func__, S__LINE__,#level, fmt, ##__VA_ARGS__);\
												}\
											} while(0)
												
#define __ADV_PRINT_BRANCH(name, level, color, colorstr, colorend, fmt,...) 	do {\
												if (AdvLog_AllNone(name, level)) {\
													break;\
												} else if (AdvLog_Dynamic_Is_On(name, level) && AdvLog_Static_Is_On(name, level) && !AdvLog_Dynamic_Is_Hiden(name, level)) {\
													AdvLog_PrintAndWrite(name, level, color, colorstr, colorend, __FILE__, __func__, S__LINE__,#level, fmt, ##__VA_ARGS__);\
												} else if(AdvLog_Dynamic_Is_On(name, level) && !AdvLog_Static_Is_On(name, level) && !AdvLog_Dynamic_Is_Hiden(name, level)) {\
													AdvLog_Print(name, level, color,__FILE__, __func__, S__LINE__,#level, fmt, ##__VA_ARGS__);\
												} else if(AdvLog_Static_Is_On(name, level)) {\
													AdvLog_Write(name, level, colorstr,colorend,__FILE__, __func__, S__LINE__,#level, fmt, ##__VA_ARGS__);\
												}\
											} while(0)

//=================================================================================================//


#define COLOR_NONE 		0

#define COLOR_RED		1
#define COLOR_GREEN		2
#define COLOR_YELLOW	3
#define COLOR_BLUE		4
#define COLOR_PURPLE	5
#define COLOR_CYAN		6
#define COLOR_GRAY		7

#define COLOR_RED_BG		11
#define COLOR_GREEN_BG		12
#define COLOR_YELLOW_BG		13
#define COLOR_BLUE_BG		14
#define COLOR_PURPLE_BG		15
#define COLOR_CYAN_BG		16
#define COLOR_WHITE_BG		17



ADVLOG_EXPORT void ADVLOG_CALL AdvLog_Init();
ADVLOG_EXPORT void ADVLOG_CALL AdvLog_Arg(int argc, char **argv);
ADVLOG_EXPORT void ADVLOG_CALL AdvLog_Default(char *arg);
ADVLOG_EXPORT void ADVLOG_CALL AdvLog_Control(int argc, char **argv);
ADVLOG_EXPORT void ADVLOG_CALL AdvLog_Uninit();

#define ADV_PRINT(level, fmt, ...)				__ADV_PRINT(level,COLOR_NONE,NULL,NULL,fmt,##__VA_ARGS__)
#define ADV_C_PRINT(level, color, fmt, ...)		__ADV_PRINT(level,color,__##color##_HTML,__##color##_END_HTML,fmt,##__VA_ARGS__)
#define ADV_PRINT_B(name, level, fmt, ...)				__ADV_PRINT_BRANCH(name,level,COLOR_NONE,NULL,NULL,fmt,##__VA_ARGS__)
#define ADV_C_PRINT_B(name, level, color, fmt, ...)		__ADV_PRINT_BRANCH(name,level,color,__##color##_HTML,__##color##_END_HTML,fmt,##__VA_ARGS__)

#define ADV_CRASH(fmt,...)  					__ADV_PRINT(LOG_CRASH,COLOR_NONE,NULL,NULL,fmt,##__VA_ARGS__)
#define ADV_C_CRASH(color,fmt,...)  			__ADV_PRINT(LOG_CRASH,color,__##color##_HTML,__##color##_END_HTML,fmt,##__VA_ARGS__)
#define ADV_CRASH_B(name,fmt,...)  						__ADV_PRINT_BRANCH(name,LOG_CRASH,COLOR_NONE,NULL,NULL,fmt,##__VA_ARGS__)
#define ADV_C_CRASH_B(name,color,fmt,...)  				__ADV_PRINT_BRANCH(name,LOG_CRASH,color,__##color##_HTML,__##color##_END_HTML,fmt,##__VA_ARGS__)

#define ADV_ERROR(fmt,...)  					__ADV_PRINT(LOG_ERROR,COLOR_NONE,NULL,NULL,fmt,##__VA_ARGS__)
#define ADV_C_ERROR(color,fmt,...)  			__ADV_PRINT(LOG_ERROR,color,__##color##_HTML,__##color##_END_HTML,fmt,##__VA_ARGS__)
#define ADV_ERROR_B(name,fmt,...)  						__ADV_PRINT_BRANCH(name,LOG_ERROR,COLOR_NONE,NULL,NULL,fmt,##__VA_ARGS__)
#define ADV_C_ERROR_B(name,color,fmt,...)  				__ADV_PRINT_BRANCH(name,LOG_ERROR,color,__##color##_HTML,__##color##_END_HTML,fmt,##__VA_ARGS__)

#define ADV_WARN(fmt,...)  						__ADV_PRINT(LOG_WARN,COLOR_NONE,NULL,NULL,fmt,##__VA_ARGS__)
#define ADV_C_WARN(color,fmt,...)  				__ADV_PRINT(LOG_WARN,color,__##color##_HTML,__##color##_END_HTML,fmt,##__VA_ARGS__)
#define ADV_WARN_B(name,fmt,...)  						__ADV_PRINT_BRANCH(name,LOG_WARN,COLOR_NONE,NULL,NULL,fmt,##__VA_ARGS__)
#define ADV_C_WARN_B(name,color,fmt,...)  				__ADV_PRINT_BRANCH(name,LOG_WARN,color,__##color##_HTML,__##color##_END_HTML,fmt,##__VA_ARGS__)

#define ADV_NOTICE(fmt,...)  					__ADV_PRINT(LOG_NOTICE,COLOR_NONE,NULL,NULL,fmt,##__VA_ARGS__)
#define ADV_C_NOTICE(color,fmt,...)				__ADV_PRINT(LOG_NOTICE,color,__##color##_HTML,__##color##_END_HTML,fmt,##__VA_ARGS__)
#define ADV_NOTICE_B(name,fmt,...)  					__ADV_PRINT_BRANCH(name,LOG_NOTICE,COLOR_NONE,NULL,NULL,fmt,##__VA_ARGS__)
#define ADV_C_NOTICE_B(name,color,fmt,...)				__ADV_PRINT_BRANCH(name,LOG_NOTICE,color,__##color##_HTML,__##color##_END_HTML,fmt,##__VA_ARGS__)

#define ADV_INFO(fmt,...)  						__ADV_PRINT(LOG_INFO,COLOR_NONE,NULL,NULL,fmt,##__VA_ARGS__)
#define ADV_C_INFO(color,fmt,...)				__ADV_PRINT(LOG_INFO,color,__##color##_HTML,__##color##_END_HTML,fmt,##__VA_ARGS__)
#define ADV_INFO_B(name,fmt,...)  						__ADV_PRINT_BRANCH(name,LOG_INFO,COLOR_NONE,NULL,NULL,fmt,##__VA_ARGS__)
#define ADV_C_INFO_B(name,color,fmt,...)				__ADV_PRINT_BRANCH(name,LOG_INFO,color,__##color##_HTML,__##color##_END_HTML,fmt,##__VA_ARGS__)

/******************************************/

#define ADV_DEBUG(fmt,...)  					__ADV_PRINT(LOG_DEBUG,COLOR_NONE,NULL,NULL,fmt,##__VA_ARGS__)
#define ADV_C_DEBUG(color,fmt,...)  			__ADV_PRINT(LOG_DEBUG,color,__##color##_HTML,__##color##_END_HTML,fmt,##__VA_ARGS__)
#define ADV_DEBUG_B(name,fmt,...)  						__ADV_PRINT_BRANCH(name,LOG_DEBUG,COLOR_NONE,NULL,NULL,fmt,##__VA_ARGS__)
#define ADV_C_DEBUG_B(name,color,fmt,...)  				__ADV_PRINT_BRANCH(name,LOG_DEBUG,color,__##color##_HTML,__##color##_END_HTML,fmt,##__VA_ARGS__)

#define ADV_TRACE(fmt,...)  					__ADV_PRINT(LOG_TRACE,COLOR_NONE,NULL,NULL,fmt,##__VA_ARGS__)
#define ADV_C_TRACE(color,fmt,...)  			__ADV_PRINT(LOG_TRACE,color,__##color##_HTML,__##color##_END_HTML,fmt,##__VA_ARGS__)
#define ADV_TRACE_B(name,fmt,...)  						__ADV_PRINT_BRANCH(name,LOG_TRACE,COLOR_NONE,NULL,NULL,fmt,##__VA_ARGS__)
#define ADV_C_TRACE_B(name,color,fmt,...)  				__ADV_PRINT_BRANCH(name,LOG_TRACE,color,__##color##_HTML,__##color##_END_HTML,fmt,##__VA_ARGS__)

#define ADV_DUMP(fmt,...)  						__ADV_PRINT(LOG_DUMP,COLOR_NONE,NULL,NULL,fmt,##__VA_ARGS__)
#define ADV_C_DUMP(color,fmt,...)  				__ADV_PRINT(LOG_DUMP,color,__##color##_HTML,__##color##_END_HTML,fmt,##__VA_ARGS__)
#define ADV_DUMP_B(name,fmt,...)  						__ADV_PRINT_BRANCH(name,LOG_DUMP,COLOR_NONE,NULL,NULL,fmt,##__VA_ARGS__)
#define ADV_C_DUMP_B(name,color,fmt,...)  				__ADV_PRINT_BRANCH(name,LOG_DUMP,color,__##color##_HTML,__##color##_END_HTML,fmt,##__VA_ARGS__)

#endif

#ifdef __cplusplus
}
#endif

#endif //__ADV_LOG_H__
