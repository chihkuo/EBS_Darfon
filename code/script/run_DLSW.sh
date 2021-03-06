#!/bin/sh

mkdir -p /tmp/test/XML/LOG
mkdir -p /tmp/test/XML/ERRLOG
mkdir -p /tmp/test/XML/ENV
mkdir -p /tmp/test/BMS
mkdir -p /tmp/test/SYSLOG

# wait network OK
sleep 30

ps | grep DLsocket.exe | grep -v grep
if [ $? != 0 ]
then
	echo "run DLsocket.exe"
	/usr/home/DLsocket.exe &
else
	echo "DLsocket.exe alive"
fi

sleep 1

ps | grep dlg320.exe | grep -v grep
if [ $? != 0 ]
then
	echo "run dlg320.exe"
	/usr/home/dlg320.exe &
else
	echo "dlg320.exe alive"
fi

sleep 60

ps | grep DataProgram.exe | grep -v grep
if [ $? != 0 ]
then
	echo "run DataProgram.exe"
	/usr/home/DataProgram.exe &
else
	echo "DataProgram.exe alive"
fi

sleep 30

ps | grep SWupdate.exe | grep -v grep
if [ $? != 0 ]
then
	echo "run SWupdate.exe"
	/usr/home/SWupdate.exe &
else
	echo "SWupdate.exe alive"
fi

sleep 30

ps | grep FWupdate.exe | grep -v grep
if [ $? != 0 ]
then
	echo "run FWupdate.exe"
	/usr/home/FWupdate.exe &
else
	echo "FWupdate.exe alive"
fi

