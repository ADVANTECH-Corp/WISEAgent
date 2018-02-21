#ifndef _READINI_H_
#define _READINI_H_

#ifdef __cplusplus
extern "C" {
#endif

//Get Current Path
int GetCurrentPath(char buf[],char *pFileName);
//Get a String From INI file
char *GetIniKeyString(char *title,char *key,char *filename);
//Get a Int Value From INI file
int GetIniKeyInt(char *title,char *key,char *filename);
//Get a Double Value From INI file
double GetIniKeyDouble(char *title,char *key,char *filename);

#ifdef __cplusplus
}
#endif

#endif