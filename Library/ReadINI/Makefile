include ../../common_config.mk

############################################################################
# Beginming of Developer Modification 
############################################################################
# for building program
application_NAME := 
# for building library
lib_NAME := libreadini
SOVERSION := 1

READINI_INC_DIR :=$(LIB_DIR)/ReadINI

program_EXT_OBJS := 
program_SUBDIRS :=
CFLAGS += -Wall -fPIC
CPPFLAGS += -D_LINUX
LDFLAGS += 
program_INCLUDE_DIRS := $(PLATFORMS_DIR) $(PLATFORM_LINUX_DIR) $(INCLUDE_DIR) $(READINI_INC_DIR)
program_LIBRARY_DIRS := $(READINI_INC_DIR)
program_LIBRARIES := 
############################################################################
# End of Developer Modification 
############################################################################

include ../../common_rule.mk
