include ../../common_config.mk
include ../../common_version.mk

############################################################################
# Beginming of Developer Modification 
############################################################################
# for building program
application_NAME := 
# for building library
lib_NAME := libSAConfig

XML2_INC_DIR := $(ROOTFS)/usr/include/libxml2

program_EXT_OBJS := $(PLATFORM_LINUX_DIR)/util_path.o $(PLATFORM_LINUX_DIR)/util_string.o $(PLATFORM_LINUX_DIR)/network.o
program_SUBDIRS :=
CFLAGS += -Wall -fPIC
CPPFLAGS += -D_LINUX
LDFLAGS += 
program_INCLUDE_DIRS := $(PLATFORM_LINUX_DIR) $(INCLUDE_DIR) $(XML2_INC_DIR)
program_LIBRARY_DIRS := 
program_LIBRARIES := xml2 pthread dl
############################################################################
# End of Developer Modification 
############################################################################

include ../../common_rule.mk

