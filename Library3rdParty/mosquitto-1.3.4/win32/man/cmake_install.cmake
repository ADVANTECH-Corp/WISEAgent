# Install script for directory: D:/workspace/SA3/Agent/CAgentLinux/Library3rdParty/mosquitto-1.3.4/man

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/mosquitto")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/man/man1" TYPE FILE FILES
    "D:/workspace/SA3/Agent/CAgentLinux/Library3rdParty/mosquitto-1.3.4/man/mosquitto_passwd.1"
    "D:/workspace/SA3/Agent/CAgentLinux/Library3rdParty/mosquitto-1.3.4/man/mosquitto_pub.1"
    "D:/workspace/SA3/Agent/CAgentLinux/Library3rdParty/mosquitto-1.3.4/man/mosquitto_sub.1"
    )
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/man/man3" TYPE FILE FILES "D:/workspace/SA3/Agent/CAgentLinux/Library3rdParty/mosquitto-1.3.4/man/libmosquitto.3")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/man/man5" TYPE FILE FILES "D:/workspace/SA3/Agent/CAgentLinux/Library3rdParty/mosquitto-1.3.4/man/mosquitto.conf.5")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/man/man7" TYPE FILE FILES
    "D:/workspace/SA3/Agent/CAgentLinux/Library3rdParty/mosquitto-1.3.4/man/mosquitto-tls.7"
    "D:/workspace/SA3/Agent/CAgentLinux/Library3rdParty/mosquitto-1.3.4/man/mqtt.7"
    )
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/man/man8" TYPE FILE FILES "D:/workspace/SA3/Agent/CAgentLinux/Library3rdParty/mosquitto-1.3.4/man/mosquitto.8")
endif()

