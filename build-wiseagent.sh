#!/bin/bash

src_home=`pwd`
tmp_build_dir=${src_home}/build
release_dir=${src_home}/Release/AgentService

precomnpile_3rdparty()
{
	cd ${src_home}/Library/Log/AdvLog
	autoreconf -i-f
	chmod 755 configure
	./configure

	cd ${src_home}/Library3rdParty/libmodbus-3.1.2
	autoreconf -i-f
	chmod 755 configure
	./configure
	make clean
	make
}

compile_agent()
{
	cd ${src_home}
	make clean
	make
	make install
}

generate_release()
{
	if [ -d "${src_home}/Library3rdParty/libmodbus-3.1.2/src/.libs" ]; then
		echo "Copy libmodbus"
		cp -avf ${src_home}/Library3rdParty/libmodbus-3.1.2/src/.libs/libmodbus.so* "${release_dir}/"
	fi
	cp -f ${src_home}/Sample/HandlerSample/module_config.xml ${release_dir}/module/
}

precomnpile_3rdparty
compile_agent
generate_release
exit 1 

