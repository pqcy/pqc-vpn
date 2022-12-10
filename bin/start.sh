#!/bin/sh
if [ "$#" -ne 4 ]; then
	echo "syntax : ./start <dummy interface> <real interface> <ip> <port>"
	echo "sample : ./start dum0 wlan0 127.0.0.1 12345"
	exit
fi
sudo ip link add "$1" type dummy
sudo ifconfig "$1" up
./vpnclient-test "$1" "$2" "$3" "$4" &
sudo dhclient -i "$1"

