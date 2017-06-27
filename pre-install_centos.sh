#!/bin/sh

# Function descript: Auto check and install the rpm's packages that you specify on CentOS
#           example: ./pre-install_chk.sh 		 --> install development packages that ${RUN_PKG_NEED} specify 

# Global variable
# You can configure following variables 
#	${RUN_PKG_NEED} is specify the depend packages to execute RMM Agent.
#	${DEVLE_PKG_NEED} is specify the depend packages to develop RMM Agent.
RUN_PKG_NEED="libxml2 openssl libcurl libXtst libX11 libmosquitto1 \
redhat-lsb-core xterm sqlite"
DEVLE_PKG_NEED="gcc gcc-c++ make \
libxml2-devel openssl-devel libcurl-devel libXtst-devel libX11-devel \
libmosquitto-devel autoconf automake libtool"
CHK_CMD=rpm
CHK_ARG=-q
INSTALL_CMD=yum
INSTALL_ARG="install -y" 
MOSQUITTO_SOURCE_URL="http://download.opensuse.org/repositories/home:/oojah:/mqtt/CentOS_CentOS-6/home:oojah:mqtt.repo"

# Do not configure this variable 
PKG_NEED_CHK=${RUN_PKG_NEED}
PKG_NEED_INSTALL=

# Color set
SETCOLOR_SUCCESS="echo -en \\033[1;32m" # green color
SETCOLOR_FAILURE="echo -en \\033[1;31m" # red color
SETCOLOR_WARNING="echo -en \\033[1;33m" # yellow color
SETCOLOR_NORMAL="echo -en \\033[0;39m"  # default color

#--------------functions-----------------
function cut_off_line()
{
	if [ $# -eq 0 ];then
		echo "===================================================================="
	else
		echo "#############################################################"
	fi
}

function mosquitto_source_import()
{
	curl ${MOSQUITTO_SOURCE_URL} > ./mosquitto_MQTT.repo
	mv -f ./mosquitto_MQTT.repo /etc/yum.repos.d/
}

function add_devel_pkg()
{
	TMP=${PKG_NEED_CHK}
	PKG_NEED_CHK="${TMP} ${DEVLE_PKG_NEED}"
}

# This function get the list of the package is not installed
function list_non_install_pkg()
{
	${CHK_CMD} ${CHK_ARG} ${PKG_NEED_CHK} | grep "is not installed" | awk '{print $2}' 
}

function check_pkg()
{
	echo "Now will check whether the following packages installed:"
	echo "  ${PKG_NEED_CHK}"
	for VAR in $(list_non_install_pkg)
	do
		TMP=${PKG_NEED_INSTALL}
		PKG_NEED_INSTALL="${TMP} ${VAR}"
	done
	if [ "x${PKG_NEED_INSTALL/ /}" != "x" ]; then
		echo "This following packages is not installed:"
		$SETCOLOR_WARNING
		echo "  ${PKG_NEED_INSTALL}"
		$SETCOLOR_NORMAL
		PKG_NEED_CHK=${PKG_NEED_INSTALL}
	fi
} 
 
function install_pkg()
{
	echo "Install ${PKG_NEED_INSTALL}:"
	cut_off_line
	${INSTALL_CMD} ${INSTALL_ARG} ${PKG_NEED_INSTALL}
	PKG_NEED_INSTALL=
}

#################### main ########################
# user check; must be root
if [ $UID -gt 0 ] &&[ "`id -un`" != "root" ]; then
	echo "You must be root !"
	exit -1
fi

add_devel_pkg

# check packages
cut_off_line
check_pkg

# insatll packages
cut_off_line
if [ "x${PKG_NEED_INSTALL// /}" != "x" ]; then

	if [ "x`echo ${PKG_NEED_INSTALL} | grep mosquitto `" != "x" ]
	then
		echo "Import mosquitto source file:"
		mosquitto_source_import
	fi
	install_pkg
else
	$SETCOLOR_SUCCESS
	echo "All the packages is installed!"
	$SETCOLOR_NORMAL
	exit 0
fi
echo 

# check packages again
cut_off_line
check_pkg
if [ "x${PKG_NEED_INSTALL// /}" != "x" ]; then
	cut_off_line "#" 
	echo -n "  Install " &&	$SETCOLOR_FAILURE
	echo -n "${PKG_NEED_INSTALL}" && $SETCOLOR_NORMAL
	echo " failed!" && $SETCOLOR_FAILURE
	echo "  You must install it manually and refer to Install manual!!"
	echo "  If not, program may run abnormal!!"
	$SETCOLOR_NORMAL
	cut_off_line "#"
else
	$SETCOLOR_SUCCESS
	echo "All the packages is installed!"
	$SETCOLOR_NORMAL
	exit 0
fi
exit 1 

