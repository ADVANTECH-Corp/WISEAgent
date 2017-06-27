#!/bin/bash

# Function : Auto check and install the deb's packages that you specify on Ubuntu
# Example  : ./pre-install_chk.sh        --> install development packages that ${RUN_PKG_NEED} specify 



# ---------------- Global variable -----------------
####################################################
## This Variables You Need Configure In Sometimes ##
####################################################
# ${RUN_PKG_NEED} is specify the depend packages to execute RMM cagent.
RUN_PKG_NEED="libxml2 libx11-6 libxext6 libxtst6 libmosquitto1 \
sqlite3 xterm ethtool"

# ${DEVLE_PKG_NEED} is specify the depend packages to develop RMM cagent.
DEVLE_PKG_NEED="gcc g++ make \
libxml2-dev libcurl4-openssl-dev libx11-dev libxtst-dev libxext-dev \
libmosquitto-dev autoconf autotools-dev build-essential libtool"

# Tell me how to import the source of mosquitto
MOSQUITTO_SOURCE_IMPORT="add-apt-repository -y ppa:mosquitto-dev/mosquitto-ppa"

# Tell me what kind of package management tools should be used
CHK_CMD=dpkg 
CHK_ARG=-l
INSTALL_CMD=apt-get 
INSTALL_ARG="install -y" 


# ====== Do not configure this variables =======
PKG_NEED_CHK=${RUN_PKG_NEED}
PKG_NEED_INSTALL=
# Color set
SETCOLOR_SUCCESS="echo -en \\033[1;32m" # green color
SETCOLOR_FAILURE="echo -en \\033[1;31m" # red color
SETCOLOR_WARNING="echo -en \\033[1;33m" # yellow color
SETCOLOR_NORMAL="echo -en \\033[0;39m"  # default color



#----------------- functions -------------------
################################################
## This Function You Need Modify In Sometimes ##
################################################
# Import mosquitto source 
import_mosquitto_source()
{
	$MOSQUITTO_SOURCE_IMPORT
	apt-get update
}

################################################
## This Function You Need Modify In Sometimes ##
################################################
# Get the list of the package is not installed
list_non_install_pkg()
{
	local list_install=`${CHK_CMD} ${CHK_ARG} ${PKG_NEED_CHK} 2>&1 | \
        awk '$1 == "ii" {n=length($2); t=index($2,":"); if(t)n=t-1; print substr($2, 1, n)}'`
	local list_non_install=$PKG_NEED_CHK
	for VAR in $list_install
	do
        #This line must be careful, blank character is useful
		list_non_install=`echo "${list_non_install} " | sed "s/${VAR}  */ /g"` 
	done
	echo ${list_non_install}
}

cut_off_line()
{
	if [ $# -eq 0 ];then
		echo "===================================================================="
	else
		echo "##############################################################"
	fi
}

add_devel_pkg()
{
	TMP=${PKG_NEED_CHK}
	PKG_NEED_CHK="${TMP} ${DEVLE_PKG_NEED}"
}

check_pkg()
{
	echo "Now will check whether the following packages installed:"
	echo "  ${PKG_NEED_CHK}"
	for VAR in $(list_non_install_pkg)
	do
		PKG_NEED_INSTALL="${PKG_NEED_INSTALL} ${VAR}"
	done
	PKG_NEED_INSTALL=`echo $PKG_NEED_INSTALL | sed 's/  */ /g'`
	if [ "x${PKG_NEED_INSTALL// /}" != "x" ]; then
		echo "This following packages is not installed:"
		$SETCOLOR_WARNING
		echo "  ${PKG_NEED_INSTALL}"
		$SETCOLOR_NORMAL
		PKG_NEED_CHK=${PKG_NEED_INSTALL}
	fi
} 
 
install_pkg()
{
	echo "Install ${PKG_NEED_INSTALL}:"
	cut_off_line
	${INSTALL_CMD} ${INSTALL_ARG} ${PKG_NEED_INSTALL}
	PKG_NEED_INSTALL=
}

#################### main ########################
# user check; must be root
if [ $UID -gt 0 ] &&[ "`id -un`" != "root" ]; then
	echo "Permission denied!"
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
		echo "Import mosquitto source:"
		import_mosquitto_source
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
	echo "  You must install it manually and refer to 'Install manual'!!"
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

