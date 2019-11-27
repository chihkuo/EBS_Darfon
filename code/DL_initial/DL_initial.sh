#!/bin/sh

# V2.5.6
# initial package dir
USB_PATH=/mnt
PACKAGE=$USB_PATH/DL_initial

# program
DATALOG=dlg320.exe
DATAPRG=DataProgram.exe
DATASWU=SWupdate.exe
DATAFWU=FWupdate.exe

# lua script
#SYSTEM=system.lua
#STAT=status.lua
#INDEX=index.lua
#NET=network.lua
#DLD=dldevice.lua
DLS=dlsetting.lua
#DLL=dllist.lua

# setting file
ST=dlsetting
#DLIST=dllist
#MLIST=ModelList
#INI=G320.ini

# boot script
RDL=run_DL.sh
RDLSW=run_DLSW.sh
CPLS=CopyLuciSetting.sh

echo "DL initial V2.5.6 script start~"

echo "Stop running progame"
/etc/init.d/run_DL.sh stop

#echo "make dir"
#mkdir /usr/home
#chmod 755 /usr/home

echo "copy program"
# program
cp $PACKAGE/$DATALOG /usr/home/
chmod 755 /usr/home/$DATALOG
cp $PACKAGE/$DATAPRG /usr/home/
chmod 755 /usr/home/$DATAPRG
cp $PACKAGE/$DATASWU /usr/home/
chmod 755 /usr/home/$DATASWU
cp $PACKAGE/$DATAFWU /usr/home/
chmod 755 /usr/home/$DATAFWU


echo "copy lua script"
# lua script
#cp $PACKAGE/$SYSTEM /usr/lib/lua/luci/controller/admin/
#chmod 755 /usr/lib/lua/luci/controller/admin/$SYSTEM
#cp $PACKAGE/$STAT /usr/lib/lua/luci/controller/admin/
#chmod 755 /usr/lib/lua/luci/controller/admin/$STAT
#cp $PACKAGE/$INDEX /usr/lib/lua/luci/controller/admin/
#chmod 755 /usr/lib/lua/luci/controller/admin/$INDEX
#cp $PACKAGE/$NET /usr/lib/lua/luci/model/cbi/admin_network/
#chmod 755 /usr/lib/lua/luci/model/cbi/admin_network/$NET
#cp $PACKAGE/$DLD /usr/lib/lua/luci/model/cbi/admin_system/
#chmod 755 /usr/lib/lua/luci/model/cbi/admin_system/$DLD
cp $PACKAGE/$DLS /usr/lib/lua/luci/model/cbi/admin_system/
chmod 755 /usr/lib/lua/luci/model/cbi/admin_system/$DLS
#cp $PACKAGE/$DLL /usr/lib/lua/luci/model/cbi/admin_system/
#chmod 755 /usr/lib/lua/luci/model/cbi/admin_system/$DLL

#echo "backup config"
#cp -r /etc/config/ /usr/home/

echo "copy setting file"
# setting file
cp $PACKAGE/$ST /etc/config/
chmod 644 /etc/config/$ST
cp $PACKAGE/$ST /usr/home/config/
chmod 644 /usr/home/config/$ST
#cp $PACKAGE/$DLIST /etc/config/
#chmod 644 /etc/config/$DLIST
#cp $PACKAGE/$DLIST /usr/home/config/
#chmod 644 /usr/home/config/$DLIST
#cp $PACKAGE/$MLIST /usr/home/
#chmod 644 /usr/home/$MLIST
#cp $PACKAGE/$MLIST /usr/home/"$MLIST"_ini
#chmod 644 /usr/home/"$MLIST"_ini
#cp $PACKAGE/$INI /usr/home/
#chmod 644 /usr/home/$INI

echo "copy boot script"
# boot script
cp $PACKAGE/$RDL /etc/init.d/
chmod 755 /etc/init.d/$RDL
cp $PACKAGE/$RDLSW /usr/home/
chmod 755 /usr/home/$RDLSW
#modify boot script
mv /etc/init.d/AutoRun.sh /etc
cp $PACKAGE/$CPLS /etc/init.d/
chmod 755 /etc/init.d/$CPLS

#echo "boot script enable"
# boot script enable
#/etc/init.d/$RDL enable

sleep 1
echo "change essid"
sed -i 's/SBC700_/DL300_/g' /etc/config/wireless
sync
#/etc/init.d/network restart
#sleep 10
#sync
#echo "network restart ok"


echo "DL initial V2.5.6 script finished."

echo "set ipv6 server disabled & reboot"
# disabled uci dhcp ipv6 server & reboot
uci set dhcp.lan.dhcpv6='disabled'
uci commit dhcp
sync
sleep 1
reboot

