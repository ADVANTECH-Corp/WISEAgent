#!/bin/sh
#./buildjson.sh
#./buildlog.sh
rm -rf out

cd AdvLog
rm -rf _install
cp Tools/AdvLog/include/AdvLog.h ..
make clean
#./configure $CONF_FLAGS
make install DESTDIR=`pwd`/_install
mkdir ../out
cp _install/usr/local/lib/libAdvJSON.a ../out
cp _install/usr/local/lib/libAdvLog.a ../out
cd ../out
ar -x libAdvJSON.a
ar -x libAdvLog.a
