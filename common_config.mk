PROJECT_ROOT_DIR := $(dir $(CURDIR)/$(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))
PLATFORMS_DIR := $(PROJECT_ROOT_DIR)/Platform
PLATFORM_LINUX_DIR := $(PLATFORMS_DIR)/Linux
LIB_DIR := $(PROJECT_ROOT_DIR)/Library
INCLUDE_DIR := $(PROJECT_ROOT_DIR)/Include
INSTALL_OUTPUT_DIR := $(PROJECT_ROOT_DIR)/Release/AgentService
LIB_3RD_DIR := $(PROJECT_ROOT_DIR)/Library3rdParty

application_DIR := Sample/SampleAgent
		   
shared_lib_NAME := libSAConfig libSAClient libSAManager libSAHandlerLoader libSAGeneralHandler libmqtthelper

program_3rdPARTY_LIBRARIES := mosquitto xml2 curl pthread

application_LIBSDIR := $(LIB_DIR)/Log \
                       $(LIB_DIR)/cJSON \
                       $(LIB_DIR)/FtpHelper \
                       $(LIB_DIR)/MD5 \
                       $(LIB_DIR)/DES \
                       $(LIB_DIR)/Base64 \
                       $(LIB_DIR)/MQTTHelper \
                       $(LIB_DIR)/SAClient \
                       $(LIB_DIR)/SAConfig \
                       $(LIB_DIR)/SAManager \
                       $(LIB_DIR)/SAHandlerLoader \
                       $(LIB_DIR)/SAGeneralHandler \
                       $(LIB_DIR)/MessageGenerator \
		       $(LIB_DIR)/ReadINI \
		       $(LIB_DIR)/HandlerKernel
                 
MODULE_DIR := $(PROJECT_ROOT_DIR)/Sample
module_LIBDIR := $(MODULE_DIR)/HandlerSample $(MODULE_DIR)/ModbusSample
shared_LIBDIR := 


