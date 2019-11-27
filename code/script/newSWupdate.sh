#!/bin/sh

sleep 1

killall -9 SWupdate.exe
sleep 1
sync

cp -f /tmp/SWupdate.exe /usr/home/
sleep 1
chmod 755 /usr/home/SWupdate.exe
sync

rm /tmp/newSWupdate.sh
rm /tmp/SWupdate.exe
sync

sleep 1
echo "change essid"
sed -i 's/SBC700_/DL300_/g' /etc/config/wireless
sync

sleep 1
#/usr/home/SWupdate.exe &
# disable uci dhcp ipv6 server & reboot
uci set dhcp.lan.dhcpv6='disabled'
uci commit dhcp
sync
reboot
