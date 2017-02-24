ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = -I../include -I../include/internal -I../../AdvJSON/inc -fPIC
AM_CXXFLAGS = -I../include -I../include/internal -I../../AdvJSON/inc -fPIC

lib_LTLIBRARIES = libAdvLog.la
libAdvLog_la_LIBADD = ../../AdvJSON/src/libAdvJSON.la
libAdvLog_la_SOURCES = AdvLog.cpp configure.cpp rollingfile.cpp advstring.cpp tool.cpp
#libAdvLog_la_LDFLAGS = -Wl,--whole-archive ../../AdvJSON/src/.libs/libAdvJSON.so -Wl,--no-whole-archive
include_HEADERS = ../include/AdvLog.h
