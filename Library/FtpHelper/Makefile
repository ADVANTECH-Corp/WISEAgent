include ../../common_config.mk

############################################################################
# Beginming of Developer Modification 
############################################################################
# for building program
application_NAME := 
# for building library
lib_NAME := libftphelper
SOVERSION := 1

CURL_INC_DIR := $(ROOTFS)/usr/include

program_EXT_OBJS := 
program_SUBDIRS :=
CFLAGS += -Wall -fPIC
CPPFLAGS += -D_LINUX -DLOG4Z
LDFLAGS += 
program_INCLUDE_DIRS := $(PLATFORM_LINUX_DIR) $(INCLUDE_DIR) $(CURL_INC_DIR) $(application_LIBSDIR)
program_LIBRARY_DIRS := $(application_LIBSDIR)
program_LIBRARIES := $(program_3rdPARTY_LIBRARIES)
############################################################################
# End of Developer Modification 
############################################################################

include ../../common_rule.mk

