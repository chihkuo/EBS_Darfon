#!/bin/sh /etc/rc.common

# Copyright (C) 2019 Techware Technology Co., Ltd.
# Paul, Chao on 2019/3/19

#chmod +x /etc/init.d/AutoRun.sh
#/etc/init.d/AutoRun.sh enable
#/etc/init.d/AutoRun.sh enabled && echo on


START=80
STOP=15

start()
{
   echo CopyLuciSetting Start
   # commands to launch application
   
   #exec 1>/dev/console  ;# redirect stdout to serial console
   #exec 2>/dev/console  ;# redirect stderr to serial console
   
   # Enable eth0 dhcp
   #dhcpcd eth0
   
   # Launch CopyLuciSetting application
   echo Launch CopyLuciSetting application
   # lua script
   SYSTEM=system.lua
   STAT=status.lua
   INDEX=index.lua
   NET=network.lua
   DLD=dldevice.lua
   DLS=dlsetting.lua
   DLL=dllist.lua
   
   
   LUCI_DST_PATH=/usr/lib/lua/luci
   LUCI_SRC_PATH=/usr/lib/lua/luci/backup
   if [ ! -e $LUCI_DST_PATH/InstallSignature.txt ]
   then
      [ -e $LUCI_SRC_PATH/controller/admin/$SYSTEM ] && cp $LUCI_SRC_PATH/controller/admin/$SYSTEM $LUCI_DST_PATH/controller/admin
      [ -e $LUCI_SRC_PATH/controller/admin/$STAT ]   && cp $LUCI_SRC_PATH/controller/admin/$STAT   $LUCI_DST_PATH/controller/admin
      [ -e $LUCI_SRC_PATH/controller/admin/$INDEX  ] && cp $LUCI_SRC_PATH/controller/admin/$INDEX  $LUCI_DST_PATH/controller/admin
      [ -e $LUCI_SRC_PATH/model/cbi/admin_network/$NET ] && cp $LUCI_SRC_PATH/model/cbi/admin_network/$NET $LUCI_DST_PATH/model/cbi/admin_network
      [ -e $LUCI_SRC_PATH/model/cbi/admin_system/$DLD  ] && cp $LUCI_SRC_PATH/model/cbi/admin_system/$DLD  $LUCI_DST_PATH/model/cbi/admin_system
      [ -e $LUCI_SRC_PATH/model/cbi/admin_system/$DLS  ] && cp $LUCI_SRC_PATH/model/cbi/admin_system/$DLS  $LUCI_DST_PATH/model/cbi/admin_system
      [ -e $LUCI_SRC_PATH/model/cbi/admin_system/$DLL  ] && cp $LUCI_SRC_PATH/model/cbi/admin_system/$DLL  $LUCI_DST_PATH/model/cbi/admin_system
      cp $LUCI_SRC_PATH/InstallSignature.txt $LUCI_DST_PATH
   fi
   
   /rom/usr/bin/ListenScan
   echo Done CopyLuciSetting application
   
}

stop() {
   echo CopyLuciSetting Stop
   # commands to kill application 
}
