include ../../common_config.mk
include ../../common_version.mk

############################################################################
# Beginming of Developer Modification 
############################################################################
# for building program
application_NAME := 
# for building library
lib_NAME := libSAClient

CJSON_INC_DIR :=$(LIB_DIR)/cJSON
LOG_INC_DIR := $(LIB_DIR)/Log

program_EXT_OBJS := $(PLATFORMS_DIR)/basequeue.o $(PLATFORM_LINUX_DIR)/util_string.o $(PLATFORM_LINUX_DIR)/util_path.o $(PLATFORM_LINUX_DIR)/util_libloader.o $(PLATFORM_LINUX_DIR)/network.o
program_SUBDIRS := 
CFLAGS += -Wall -fPIC
CPPFLAGS += -D_LINUX
LDFLAGS += -Wl,-rpath,./ -Wl,-rpath,/usr/local/AgentService/ 
program_INCLUDE_DIRS := $(PLATFORMS_DIR) $(PLATFORM_LINUX_DIR) $(INCLUDE_DIR) $(CJSON_INC_DIR) $(LOG_INC_DIR) $(LIB_DIR)/DES $(LIB_DIR)/Base64 $(LIB_DIR)/MQTTHelper
program_LIBRARY_DIRS := $(CJSON_INC_DIR) $(LOG_INC_DIR) $(LIB_DIR)/Base64 $(LIB_DIR)/DES $(LIB_DIR)/MQTTHelper
program_LIBRARIES := Base64 DES Log cJSON mqtthelper pthread dl
############################################################################
# End of Developer Modification 
############################################################################

include ../../common_rule.mk

