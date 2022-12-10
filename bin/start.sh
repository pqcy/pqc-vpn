#!/bin/sh
sudo ip link add dum0 type dummy
sudo ip link set dum0 addr 00:00:00:11:11:11
sudo ifconfig dum0 up
