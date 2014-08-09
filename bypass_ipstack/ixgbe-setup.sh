#!/bin/sh              

sudo ifconfig $1 up
sudo ethtool -A $1 autoneg off rx off tx off
sudo ethtool -s $1 speed 10000 duplex full
sudo irq-affinity -a naive $1
