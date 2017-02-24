/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2015/07/07 by Fred Chang									*/
/* Modified Date: 2015/07/07 by Fred Chang									*/
/* Abstract     : Advantech Logging Library    						        */
/* Reference    : None														*/
/****************************************************************************/
#ifndef __ADV_STRING_H__
#define __ADV_STRING_H__


class advstring {
public:
	advstring(int size);
	~advstring();
	inline void clear();
	const char *str();
	int size();
	
	void insert(int clear = 0, ...);
	void assign(const char *fmt, va_list ap);
	
	advstring& operator<< (char *string);
	advstring& operator<< (const char *string);
	//advstring& operator<< (int number);
	
private:
	char *_buffer;
	int _size;
	int _currentPos;
	
	
	
	
};

#endif //__ADV_STRING_H__