include ../../common_config.mk

############################################################################
# Beginming of Developer Modification 
############################################################################
# for building program
application_NAME := 
# for building library
lib_NAME := libLog
SOVERSION := 1
#LOG4Z_INC_DIR := $(LIB_3RD_DIR)/log4z-3.2.0
ADVLOG_INC_DIR := AdvLog/Tools/AdvLog/include
program_EXT_OBJS := $(PLATFORM_LINUX_DIR)/util_path.o out/*.o
program_SUBDIRS := 
CFLAGS += -Wall -fPIC
CPPFLAGS += -Wall -fPIC
CXXFLAGS += -fPIC
LDFLAGS += 
#program_INCLUDE_DIRS := $(PLATFORM_LINUX_DIR) $(LOG4Z_INC_DIR)
program_INCLUDE_DIRS := $(PLATFORM_LINUX_DIR) $(ADVLOG_INC_DIR)
program_LIBRARY_DIRS :=
program_LIBRARIES := 
############################################################################
# End of Developer Modification 
############################################################################

include ../../common_rule.mk

out/*.o:
	./build.sh

