#!/bin/sh

UPDATE_DIR=/tmp/update
SYSTEM=system.lua
DLD=dldevice.lua
DLS=dlsetting.lua
DLL=dllist.lua
ST=dlsetting
DLIST=dllist
MLIST=ModelList

cp $UPDATE_DIR/$SYSTEM /usr/lib/lua/luci/controller/admin/
chmod 755 /usr/lib/lua/luci/controller/admin/$SYSTEM
cp $UPDATE_DIR/$DLD /usr/lib/lua/luci/model/cbi/admin_system/
chmod 755 /usr/lib/lua/luci/model/cbi/admin_system/$DLD
cp $UPDATE_DIR/$DLS /usr/lib/lua/luci/model/cbi/admin_system/
chmod 755 /usr/lib/lua/luci/model/cbi/admin_system/$DLS
cp $UPDATE_DIR/$DLL /usr/lib/lua/luci/model/cbi/admin_system/
chmod 755 /usr/lib/lua/luci/model/cbi/admin_system/$DLL
cp $UPDATE_DIR/$ST /etc/config/
chmod 644 /etc/config/$ST
cp $UPDATE_DIR/$ST /usr/home/
chmod 644 /usr/home/$ST
cp $UPDATE_DIR/$DLIST /etc/config/
chmod 644 /etc/config/$DLIST
cp $UPDATE_DIR/$MLIST /usr/home/
chmod 644 /etc/config/$MLIST

rm /tmp/luci-indexcache
rm /tmp/luci-modulecache/*
sync

