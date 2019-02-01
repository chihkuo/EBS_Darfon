#!/bin/sh

# initial package dir
USB_PATH=/tmp/run/mountd/sda1
PACKAGE=$USB_PATH/DL_initial

# program
DATALOG=dlg320.exe
DATAPRG=DataProgram.exe
DATASWU=SWupdate.exe

# lua script
SYSTEM=system.lua
STAT=status.lua
NET=network.lua
INDEX=index.lua
LST=luci_statistics.lua
DLD=dldevice.lua
DLS=dlsetting.lua
DLL=dllist.lua

# setting file
ST=dlsetting
DLIST=dllist
MLIST=ModelList
INI=G320.ini

# boot script
RDL=run_DL.sh
RDLSW=run_DLSW.sh

echo "DL initial script start~"

echo "make dir"
mkdir /usr/home
chmod 755 /usr/home

echo "copy program"
# program
cp $PACKAGE/$DATALOG /usr/home/
chmod 755 /usr/home/$DATALOG
cp $PACKAGE/$DATAPRG /usr/home/
chmod 755 /usr/home/$DATAPRG
cp $PACKAGE/$DATASWU /usr/home/
chmod 755 /usr/home/$DATASWU

echo "copy lua script"
# lua script
cp $PACKAGE/$SYSTEM /usr/lib/lua/luci/controller/admin/
chmod 755 /usr/lib/lua/luci/controller/admin/$SYSTEM
cp $PACKAGE/$STAT /usr/lib/lua/luci/controller/admin/
chmod 755 /usr/lib/lua/luci/controller/admin/$STAT
cp $PACKAGE/$NET /usr/lib/lua/luci/controller/admin/
chmod 755 /usr/lib/lua/luci/controller/admin/$NET
cp $PACKAGE/$INDEX /usr/lib/lua/luci/controller/admin/
chmod 755 /usr/lib/lua/luci/controller/admin/$INDEX
cp $PACKAGE/$LST /usr/lib/lua/luci/controller/luci_statistics/
chmod 755 /usr/lib/lua/luci/controller/luci_statistics/$LST
cp $PACKAGE/$DLD /usr/lib/lua/luci/model/cbi/admin_system/
chmod 755 /usr/lib/lua/luci/model/cbi/admin_system/$DLD
cp $PACKAGE/$DLS /usr/lib/lua/luci/model/cbi/admin_system/
chmod 755 /usr/lib/lua/luci/model/cbi/admin_system/$DLS
cp $PACKAGE/$DLL /usr/lib/lua/luci/model/cbi/admin_system/
chmod 755 /usr/lib/lua/luci/model/cbi/admin_system/$DLL

echo "copy setting file"
# setting file
cp $PACKAGE/$ST /etc/config/
chmod 644 /etc/config/$ST
cp $PACKAGE/$ST /usr/home/
chmod 644 /usr/home/$ST
cp $PACKAGE/$DLIST /etc/config/
chmod 644 /etc/config/$DLIST
cp $PACKAGE/$MLIST /usr/home/
chmod 644 /usr/home/$MLIST
cp $PACKAGE/$INI /usr/home/
chmod 644 /usr/home/$INI

echo "copy boot script"
# boot script
cp $PACKAGE/$RDL /etc/init.d/
chmod 755 /etc/init.d/$RDL
cp $PACKAGE/$RDLSW /usr/home/
chmod 755 /usr/home/$RDLSW

echo "boot script enable"
# boot script enable
/etc/init.d/$RDL enable

echo "DL initial script finished."

