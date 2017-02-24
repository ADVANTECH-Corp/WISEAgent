include ../../common_config.mk
include ../../common_version.mk

############################################################################
# Beginming of Developer Modification 
############################################################################
# for building program
application_NAME := 
# for building library
lib_NAME := libHandlerKernel

CJSON_INC_DIR :=$(LIB_DIR)/cJSON
LOG_INC_DIR := $(LIB_DIR)/Log
MSGGEN_INC_DIR := $(LIB_DIR)/MessageGenerator

program_EXT_OBJS := $(PLATFORM_LINUX_DIR)/util_string.o $(PLATFORM_LINUX_DIR)/util_path.o $(PLATFORMS_DIR)/basequeue.o
program_SUBDIRS := 
CFLAGS += -Wall -fPIC
CPPFLAGS += -D_LINUX
LDFLAGS += -Wl,-rpath,./,-lrt -lm -ldl
program_INCLUDE_DIRS := $(PLATFORMS_DIR) $(PLATFORM_LINUX_DIR) $(INCLUDE_DIR) $(CJSON_INC_DIR) $(LOG_INC_DIR) $(MSGGEN_INC_DIR)
program_LIBRARY_DIRS := $(CJSON_INC_DIR) $(LOG_INC_DIR) $(MSGGEN_INC_DIR)
program_LIBRARIES := Log cJSON msggen pthread
############################################################################
# End of Developer Modification 
############################################################################

include ../../common_rule.mk

