ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = -I../inc -fPIC
AM_CXXFLAGS = -I../inc -fPIC

lib_LTLIBRARIES = libAdvJSON.la 
#libAdvCJSON.la
libAdvJSON_la_SOURCES = AdvJSON.cpp AdvCollection.cpp AdvVariousType.cpp AdvCJSON.c jsontool.c jsonrule.c jsonparser.c jsonprint.c jsoncollection.c jsoncreator.c
libAdvJSON_la_CFLAGS = -I../inc -fPIC
#libAdvCJSON_la_SOURCES = AdvCJSON.c jsontool.c jsonrule.c jsonparser.c jsonprint.c jsoncollection.c jsoncreator.c
#libAdvCJSON_la_CFLAGS = -I../inc -fPIC

include_HEADERS = ../inc/AdvJSON.h ../inc/AdvJSONParallel.h ../inc/AdvJSONCreator.h ../inc/AdvJSONCreatorParallel.h ../inc/AdvVariousType.h ../inc/AdvCollection.h
