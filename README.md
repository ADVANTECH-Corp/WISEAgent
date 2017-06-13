## WISEAgent
----
WISEAgent Framework V3.X

## DESCRIPTION
----

WISE Agnet – a software development framework to communicate between device and RMM Server.
Advantech provides a software development framework to communicate and exchange information between a device and RMM Server, called a WISE Agent framework.
The WISE Agent framework provides a rich set of user-friendly, intelligent and integrated interfaces, which speeds development, enhances security and makes agent application easier and simpler to communicate with RMM Server.
The framework has three benefits:

Standardization - The communication protocol is based on the MQTT protocol to communicate and exchange data with WISE-PaaS/RMM Server. The IoT sensor data report format is following the IPSO Spec. in JSON format.
Portability - Whole framework is written in C language and follow the ANSI C Standard, that C compilers are available for most systems and are often the first compiler provided for a new system.
Scalability - The WISE Agent Framework is functional partitioning into discrete scalable, reusable modules, and plug & playable.

For scalability, Advantech implements several Software Modules to access sensor data or control target device, we called Plugins (or Handlers we called before).

Each plugin can be designed to handle specific jobs, such as:

Sensor Plugin: the plugin access sensor data through Sensor driver or 3rd party library, or
Remote Control Plugin:  the plugin execute remote command on target device.
Based on those Plugins, Advantech also defined a set of APIs, called Plugin (Handler) APIs, to support custom plugins implementation.

User can implement their own Handler with these Plugin (Handler) APIs to access their sensor data or control their devices and plugged by WISE Agent to communicate with RMM Server.

## OVERVIEW
----

 The WISE Agent includes:
 
 Library:
 
  * libSAClient.so
    - Communicate with RMM Server version 3.x
	 
  * libSAManager.so
    - Integrate whole RMM Agent library
	 
  * libSAHandlerLoader.so:
    - Dynamic load and manage Handlers

Application:     
  * Sample\SampleClient
    - Sample application for Windows only
     
  * Sample\SampleAgent
    - Sample application for both Windows and Linux
     
Plugins:
 * Sample\HandlerSample:
   - Sample Plugin to read a JSON file and upload to RMM Server for both Windows and Linux.

 * Sample\SampleHandler:
   - Sample Plugin to read a JSON file and upload to RMM Server for Windows only.
	 
 * Sample\Modbus_Handler:
   - Network Monitor Handler for network status report.
	 
Decuments:
 * doc\WISE-PaaS RMM 3.1_WISE-Agent_Command_Format_DG_V1.1.pdf:
   - document for WISE Agent Framework command format.
	 
 * doc\WISE-PaaS RMM 3.1_WISE-Agent_DG_V1.3.pdf:
   - document for WISE Agent Framework programming guide.
	 
 * doc\WISE-PaaS RMM 3.1_Agent_UM.pdf
   - document for RMM Agent User Manual. 
	 
## PATENTS
----

## COMPILATION
----

 * To compile this package under a Unix, user need install the following libraries:
   - libXml2-2.7.8
   - openssl-1.0.1h
   - curl-7.37.1
   - mosquitto-1.3.2
   - autoconf
   - automake
   - make
   - libx86
   
 * Linux Build Procedure:
   - cd Library/Log/AdvLog/
   - autoreconf -if
   - ./configure
   - cd ../../..
   - cd Library3rdParty/libmodbus-3.1.2/
   - autoreconf -if
   - ./configure
   - make
   - cd ../..
   - make

  
## Hardware requirements
----

* CPU architecture
  - x86
  - ARM
 
## OS
----

 * Windows
   - XP, 7, 8, 10

 * Linux
   - Ubuntu ( 14.04.2 X64, 16.04 X86 )
   - CentOS 6.5 X86
   - OpenSUSE 12.3 X86 
   - Yocto ( 1.5.3 Dora i.MX6)
   - Yocto ( 1.7 X86 )
 
## PROBLEMS
----

## SUPPORT
----

 * [Advantech IoT Developer Forum](http://iotforum.advantech.com/)
 * [WISE Agent WIKI](http://ess-wiki.advantech.com.tw/view/WISE-Agent)
 
## License
----

Copyright (c) Advantech Corporation. All rights reserved.
