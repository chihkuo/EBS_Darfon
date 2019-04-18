#!/bin/sh /etc/rc.common

START=99
STOP=10

start()
{
	echo "run_DLSW.sh"
	/usr/home/run_DLSW.sh &
}

stop()
{
	echo "call run_SL.sh stop"
	killall -9 FWupdate.exe SWupdate.exe DataProgram.exe dlg320.exe
}

