#!/bin/sh

UPDATE_DIR=/tmp/update
DLD=dldevice.lua
DLS=dlsetting.lua
ST=dlsetting

cp $UPDATE_DIR/$DLD /usr/lib/lua/luci/model/cbi/admin_system/
chmod 755 /usr/lib/lua/luci/model/cbi/admin_system/$DLD
cp $UPDATE_DIR/$DLS /usr/lib/lua/luci/model/cbi/admin_system/
chmod 755 /usr/lib/lua/luci/model/cbi/admin_system/$DLS
cp $UPDATE_DIR/$ST /etc/config/
chmod 644 /etc/config/$ST
cp $UPDATE_DIR/$ST /usr/home/
chmod 644 /usr/home/$ST

