#!/bin/sh

UPDATE_DIR=/tmp/update
SYSTEM=system.lua
DLD=dldevice.lua
DLS=dlsetting.lua
DLL=dllist.lua
NET=network.lua
ST=dlsetting
DLIST=dllist
MLIST=ModelList

STAT=status.lua
INDEX=index.lua
#LST=luci_statistics.lua
RUNDLSW=run_DLSW.sh

cp $UPDATE_DIR/$SYSTEM /usr/lib/lua/luci/controller/admin/
chmod 755 /usr/lib/lua/luci/controller/admin/$SYSTEM
cp $UPDATE_DIR/$STAT /usr/lib/lua/luci/controller/admin/
chmod 755 /usr/lib/lua/luci/controller/admin/$STAT
cp $UPDATE_DIR/$INDEX /usr/lib/lua/luci/controller/admin/
chmod 755 /usr/lib/lua/luci/controller/admin/$INDEX
#cp $UPDATE_DIR/$LST /usr/lib/lua/luci/controller/luci_statistics/
#chmod 755 /usr/lib/lua/luci/controller/luci_statistics/$LST
cp $UPDATE_DIR/$DLD /usr/lib/lua/luci/model/cbi/admin_system/
chmod 755 /usr/lib/lua/luci/model/cbi/admin_system/$DLD
cp $UPDATE_DIR/$DLS /usr/lib/lua/luci/model/cbi/admin_system/
chmod 755 /usr/lib/lua/luci/model/cbi/admin_system/$DLS
cp $UPDATE_DIR/$DLL /usr/lib/lua/luci/model/cbi/admin_system/
chmod 755 /usr/lib/lua/luci/model/cbi/admin_system/$DLL
cp $PACKAGE/$NET /usr/lib/lua/luci/model/cbi/admin_network/
chmod 755 /usr/lib/lua/luci/model/cbi/admin_network/$NET
cp $UPDATE_DIR/$ST /etc/config/
chmod 644 /etc/config/$ST
cp $UPDATE_DIR/$ST /usr/home/config/
chmod 644 /usr/home/config/$ST
cp $UPDATE_DIR/$DLIST /etc/config/
chmod 644 /etc/config/$DLIST
cp $UPDATE_DIR/$MLIST /usr/home/"$MLIST"_ini
chmod 644 /usr/home/"$MLIST"_ini
cp $UPDATE_DIR/$RUNDLSW /usr/home/
chmod 755 /usr/home/$RUNDLSW

rm /tmp/luci-indexcache
rm /tmp/luci-modulecache/*
sync

SWupdatesh=newSWupdate.sh
SWupdate=SWupdate.exe
if [ -f $UPDATE_DIR/$SWupdate ]
then
	cp $UPDATE_DIR/$SWupdatesh /tmp/
	chmod 755 /tmp/$SWupdatesh
	cp $UPDATE_DIR/$SWupdate /tmp/
	chmod 755 /tmp/$SWupdate
fi

