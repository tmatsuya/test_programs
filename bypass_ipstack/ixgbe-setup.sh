#!/bin/sh              

sudo ifconfig $1 up
sudo ethtool -A $1 autoneg off rx off tx off
sudo ethtool -s $1 speed 10000 duplex full
sudo ethtool -K $1 gro off
sudo ethtool -k $1
sudo ethtool -G $1 rx 4096
sudo ethtool -g $1
#sudo systemctl stop firewalld
#sudo systemctl stop iptables
#sudo ethtool -C $1 rx-usecs 50
#sudo setpci -v -d 8086:10fb e6.b=22
#sudo setpci -v -d 8086:10fb e6.b=2e
#sudo lspci -s 01:00 -xxx
