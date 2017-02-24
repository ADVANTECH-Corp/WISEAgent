#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "jsontool.h"
#include "jsonrule.h"


int IsString(const char *json) {
	int len = 0;
	char *s;
	char *e;
	char *b = NULL;
	char *p = (char *)json;
	char *pp;
	p = skip(p);
	if(*p != '\"') return 0;
	s = p; //string start;
	p++;
	pp = p;
	p = strchr(p,'\"');
	if(p == NULL) return (int)((long)pp - (long)s);
	else {
		b = p-1;
		while(*b == '\\') {
			p++;
			pp = p;
			p = strchr(p,'\"');
			if(p == NULL) return (int)((long)pp - (long)s);
			b = p-1;
		}
	}
	e = p; //string end;
	p++;
	p = skip(p);
	if(*p == ':' || *p == ',' || *p == '}' || *p == ']') {
		len = (int)((long)e - (long)s) + 1;
		return len;
	} else return (int)((long)p - (long)s);
}

int IsNumber(const char *json) {
	int len = 0;
	char *p = (char *)json;
	p = skip(p);
	if(*p != '-' && !isdigit(*p)) return 0;
	len = strspn(p,"+-01234567890.eE");
	p+=len;
	p = skip(p);
	if(*p == ',' || *p == '}' || *p == ']') {
		return len;
	} else return 0;
}

int IsObject(const char *json) {
	int len = 0;
	int eleLen = 0;
	char *s;
	char *e;
	char *p = (char *)json;
	p = skip(p);
	if(*p != '{') return 0;
	s = p; //object start
	p++;
	p = skip(p);
	if(*p == '}') {
		e = p; //object end
		p++;
		p = skip(p);
		if(*p == ',' || *p == '}' || *p == ']' || *p == '\0') {
			len = (int)((long)e - (long)s) + 1;
			return len;
		} else return (int)((long)p - (long)s);
	}
	eleLen = IsString(p);
	if(eleLen == 0) return 0;
	p += eleLen;
	p = skip(p);
	if(*p != ':') return (int)((long)p - (long)s);
	p++;
	p = skip(p);
	eleLen = IsValue(p);
	if(eleLen == 0) return 0;
	p += eleLen;
	p = skip(p);
	
	while(*p == ',') {
		p++;
		p = skip(p);
		eleLen = IsString(p);
		if(eleLen == 0) return 0;
		p += eleLen;
		p = skip(p);
		if(*p != ':') return (int)((long)p - (long)s);
		p++;
		p = skip(p);
		eleLen = IsValue(p);
		if(eleLen == 0) return 0;
		p += eleLen;
		p = skip(p);
	}
	
	if(*p == '}') {
		e = p; //object end
		p++;
		p = skip(p);
		if(*p == ',' || *p == '}' || *p == ']' || *p == '\0') {
			len = (int)((long)e - (long)s) + 1;
			return len;
		} else return (int)((long)p - (long)s);
	} else return (int)((long)p - (long)s);
	
	
}

int IsArray(const char *json) {
	int len = 0;
	int eleLen = 0;
	char *s;
	char *e;
	char *p = (char *)json;
	p = skip(p);
	if(*p != '[') return 0;
	s = p; //array start
	p++;
	p = skip(p);
	if(*p == ']') {
		e = p; //array end
		p++;
		p = skip(p);
		if(*p == ',' || *p == '}' || *p == ']') {
			len = (int)((long)e - (long)s) + 1;
			return len;
		} else return (int)((long)p - (long)s);
	}
	eleLen = IsValue(p);
	if(eleLen == 0) return (int)((long)p - (long)s);
	p += eleLen;
	p = skip(p);
	
	while(*p == ',') {
		p++;
		p = skip(p);
		eleLen = IsValue(p);
		if(eleLen == 0) return (int)((long)p - (long)s);
		p += eleLen;
		p = skip(p);
	}
	
	if(*p == ']') {
		e = p; //array end
		p++;
		p = skip(p);
		if(*p == ',' || *p == '}' || *p == ']') {
			len = (int)((long)e - (long)s) + 1;
			return len;
		} else return (int)((long)p - (long)s);
	} else return (int)((long)p - (long)s);
	
}

int IsValue(const char *json) {
	int len = 0;
	char *p = (char *)json;
	p = skip(p);

	switch(*p) {
		case 't':
			if(strncmp(p,"true",4) == 0) {
				p+=4;
				p = skip(p);
				if(*p == ',' || *p == '}' || *p == ']') {
					return 4;
				} else return 0;
			} else return 0;
		case 'f':
			if(strncmp(p,"false",5) == 0) {
				p+=5;
				p = skip(p);
				if(*p == ',' || *p == '}' || *p == ']') {
					return 5;
				} else return 0;
			} else return 0;
		break;
		case 'n':
			if(strncmp(p,"null",4) == 0) {
				p+=4;
				p = skip(p);
				if(*p == ',' || *p == '}' || *p == ']') {
					return 4;
				} else return 0;
			} else return 0;
		default:
			len += IsString(p);
			if(len != 0) return len;
			len += IsObject(p);
			if(len != 0) return len;
			len += IsArray(p);
			if(len != 0) return len;
			len += IsNumber(p);
			if(len != 0) return len;
			break;
	}

	return len;
}


