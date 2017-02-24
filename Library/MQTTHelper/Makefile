include ../../common_config.mk
include ../../common_version.mk

############################################################################
# Beginming of Developer Modification 
############################################################################
# for building program
application_NAME := 
# for building library
lib_NAME := libmqtthelper

program_EXT_OBJS := $(PLATFORM_LINUX_DIR)/util_path.o
program_SUBDIRS := 
CFLAGS += -Wall -fPIC
CPPFLAGS += -D_LINUX
LDFLAGS += 
program_INCLUDE_DIRS := $(PLATFORM_LINUX_DIR) $(INCLUDE_DIR)
program_LIBRARY_DIRS := 
program_LIBRARIES := mosquitto pthread dl
############################################################################
# End of Developer Modification 
############################################################################

include ../../common_rule.mk

