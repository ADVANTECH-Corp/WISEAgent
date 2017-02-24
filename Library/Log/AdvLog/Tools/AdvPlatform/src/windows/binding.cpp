#include "binding.h"
#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>

ssize_t ADVPLAT_CALL getline(char **lineptr, size_t *n, void *stream) {
	std::filebuf buf((FILE *)stream);
	std::istream base(&buf);
	std::string sbuf;

	if (!base.good()) return -1;
	if (feof((FILE *)stream)) return -1;

	std::getline(base, sbuf);
	*n = sbuf.size() + 1;

	if (*lineptr != NULL) *lineptr = (char *)realloc(*lineptr, *n);
	else *lineptr = (char *)malloc(*n);
	strncpy(*lineptr, sbuf.c_str(), *n);

	return *n;
}

