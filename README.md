# WISEAgent
WISEAgent Framework V3.2

 Copyright (c) 2013-2017 The WISE Agent Project
 All rights reserved.

 # DESCRIPTION
 -----------

 

 # OVERVIEW
 --------

 The RMM Agent includes:
 
Library:
 libSAClient.so:
     Communicate with RMM Server
	 
 libSAGatherInfo.so:
     Gathering OS and Platform information.
	 
 libSAManager.so:
	 Integrate whole RMM Agent library
	 
 libSAHandlerLoader.so:
	 Dynamic load and manage Handlers

Application:     
 Sample\SampleClient
     Sample application for Windows only
     
 Sample\SampleAgent
     Sample application for both Windows and Linux
     
Confugration File: 
 agent_config.xml:
     Agent configuration in XML format. Include:
	 BaseSettings:
    <RunMode>: preserve for UI control, Standalone/Remote
	  <AutoStart>: autostart flag
    <ServerIP>: server IP
		<ServerPort>: server listen port, default is 1883
		<ConnAuth>: MQTT connection authentication string, ecvrypt with DES and Base64.
		<TLSType>: MQTT TLS type, 0: disable, 1: TLS, 2: TLS_PSK
		<CAFile>: Certificate Authority File in TLS mode.
		<CAPath>: Certificate Authority Path in TLS mode.
		<CertFile>: Certificate signing file in TLS mode.
		<KeyFile>: Certificate key in TLS mode.
		<CertPW>: Certificate password in TLS mode.
		<PSK>: Pre-shared-key in TLS-PSK mode.
		<PSKIdentify>: Identify of PSK in TLS-PSK mode.
		<PSKCiphers>: PSK supported Ciphers in TLS-PSK mode.
		
	Profiles:
		<DeviceName>: Device Name shown on Agent.
		<SWVersion>: Agent application version.
		<DevID>: Device ID,
		<SN>: serial number,
		<DevType>: Device Type, such as: IPC or Gateway.
		<Product>: Product name,
		<Manufacture>: Manufacture name.
		<WorkDir>: Application working directory.
		<UserName>: Login account, default = anonymous
		<UserPassword>: Login password, default is empty
	
	Customize Info:
		<KVMMode>: KVM Mode, 1. default: embedded UltraVNC server
							 2. custom: custom VNC server
							 3. disable: not support
		<CustVNCPwd>: custom vcn login password
		<CustVNCPort>: custom vnc server listen port.
		
		<AmtEn>: Enable Intel AMT flag.
		<AmtID>: Intel AMT login account.
		<AmtPwd>: Intel AMT login password
	
	
 modlue\module_config.xml:
     Modules configuration in XML format. Include:
     <ModuleNum>: current supported module count.
     <ModuleName#>: # module name.
     <ModulePath#>: # module path.
     <ModuleEnable#>: # module enable flag.

Plugins:
 Sample\HandlerSample:
     Sample Plugin to read a JSON file and upload to RMM Server for both Windows and Linux.

 Sample\SampleHandler:
     Sample Plugin to read a JSON file and upload to RMM Server for Windows only.
	 
 Sample\Modbus_Handler:
     Network Monitor Handler for network status report.
	 
 module\PowerOnOffHandler.so:
     Power Control Handler for remote power control.
	 
 module\ProcessorMonitorHandler.so:
     Access Modbus Device with modbus tcp or RTU protocol and report to RMM Server.
	 
Decuments:
 doc\license:
     	 
 doc\README:
     document for this WISEAgent Framework.
	 
 doc\WISE-PaaS RMM 3.1_WISE-Agent_Command_Format_DG_V1.1.pdf:
     document for WISE Agent Framework command format.
	 
 doc\WISE-PaaS RMM 3.1_WISE-Agent_DG_V1.3.pdf:
     document for WISE Agent Framework programming guide.
	 
 doc\WISE-PaaS RMM 3.1_Agent_UM.pdf
     document for RMM Agent User Manual. 
	 
 # PATENTS
 -------

 

 # INSTALLATION
 ------------

 To install this package under a Unix, user need install the following libraries:
   libXml2-2.7.8
   openssl-1.0.1h
   curl-7.37.1
   mosquitto-1.3.2
   libjpeg
   libX11
   autoconf
   automake
   make
   libx86
   libxtst
   xterm

	
 # PROBLEMS
 --------


 # SUPPORT
 -------

 You might install it like this:
 In *.run
	#sudo ./rmmagent-platform_name-version.run
In *.tar.gz
	#tar -zxf rmmagent-platform_name-version.run.tar.gz
	# cd Wrapper
	#sudo ./rmmagent-platform_name-version.run
 Please reference to install manuall to get operation commands. 
