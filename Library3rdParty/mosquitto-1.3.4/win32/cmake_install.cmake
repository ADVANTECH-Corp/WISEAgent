# Install script for directory: D:/workspace/SA3/Agent/CAgentLinux/Library3rdParty/mosquitto-1.3.4

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "mosquitto")
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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/." TYPE FILE FILES
    "D:/workspace/SA3/Agent/CAgentLinux/Library3rdParty/mosquitto-1.3.4/mosquitto.conf"
    "D:/workspace/SA3/Agent/CAgentLinux/Library3rdParty/mosquitto-1.3.4/aclfile.example"
    "D:/workspace/SA3/Agent/CAgentLinux/Library3rdParty/mosquitto-1.3.4/pskfile.example"
    "D:/workspace/SA3/Agent/CAgentLinux/Library3rdParty/mosquitto-1.3.4/pwfile.example"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("D:/workspace/SA3/Agent/CAgentLinux/Library3rdParty/mosquitto-1.3.4/win32/lib/cmake_install.cmake")
  include("D:/workspace/SA3/Agent/CAgentLinux/Library3rdParty/mosquitto-1.3.4/win32/client/cmake_install.cmake")
  include("D:/workspace/SA3/Agent/CAgentLinux/Library3rdParty/mosquitto-1.3.4/win32/src/cmake_install.cmake")
  include("D:/workspace/SA3/Agent/CAgentLinux/Library3rdParty/mosquitto-1.3.4/win32/man/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

file(WRITE "D:/workspace/SA3/Agent/CAgentLinux/Library3rdParty/mosquitto-1.3.4/win32/${CMAKE_INSTALL_MANIFEST}" "")
foreach(file ${CMAKE_INSTALL_MANIFEST_FILES})
  file(APPEND "D:/workspace/SA3/Agent/CAgentLinux/Library3rdParty/mosquitto-1.3.4/win32/${CMAKE_INSTALL_MANIFEST}" "${file}\n")
endforeach()
