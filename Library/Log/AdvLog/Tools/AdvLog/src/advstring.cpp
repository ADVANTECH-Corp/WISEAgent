#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "advstring.h"
#include "fplatform.h"

advstring::advstring(int size) {
	this->_size = size;
	_buffer = (char *)malloc(size);
	clear();
}

advstring::~advstring() {
	free(_buffer);
}

inline void advstring::clear() {
	_buffer[0] = 0;
	_currentPos = 0;
}

const char *advstring::str() {
	return _buffer;
}

int advstring::size() {
	return _currentPos;
}

void advstring::insert(int clear, ...) {
	
	if(clear) {
		this->clear();
	}

	int len;
	char *val;
	//char *temp;
	va_list vl;
	va_start(vl,clear);
	val=va_arg(vl,char *);
	while(val != NULL) {
		len = strlen(val);
		if (_currentPos + len + 1 > _size) {
			_size = _currentPos + len + 1;
			_buffer = (char*)realloc(_buffer, _size);
		}
		memcpy(&_buffer[_currentPos],val,len);
		_currentPos += len;
		_buffer[_currentPos] = 0;
		val=va_arg(vl,char *);
	}
	
	va_end(vl);
}

void advstring::assign(const char *fmt, va_list ap) {
	
	va_list args;
	va_copy(args, ap);
	
	int len = vsnprintf(NULL, 0, fmt, ap);
    if(len > _size) {
		_size = len;
		_buffer = (char*)realloc(_buffer, _size);
		
	}
	
	vsnprintf(_buffer, _size, fmt, args);
	_currentPos = len;
	_buffer[len] = 0;
}

advstring& advstring::operator<< (char *string) {
	int len = strlen(string);
	if(_currentPos + len + 1 > _size) {
		_size = _currentPos + len + 1;
		_buffer = (char*)realloc(_buffer, _size);
	}

	memcpy(&_buffer[_currentPos],string,len);
	_currentPos += len;
	_buffer[_currentPos] = 0;
	return *this;
}

advstring& advstring::operator<< (const char *string) {
	*this << (char *)string;
	return *this;
}
