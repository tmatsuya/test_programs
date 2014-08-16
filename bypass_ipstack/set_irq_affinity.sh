#!/bin/bash
#
# = RSS対応NICの各キューをCPUコアに割り当てるスクリプト
#
# Author::  harukasan <harukasan@pixiv.com>
# License:: MIT
#
# == Ref
# http://d.hatena.ne.jp/sfujiwara/20121221/1356084456のコメント欄
#
# == NOTE
# irqbalaceを使うとよくわからないタイミングでコア割り当てを
# 変更されるので、起動しないようにしておく
#

# affinityを書き込む
# @param irq  integer IRQ番号
# @param mask integer ビットマスク
set_affinity()
{
    local irq=$1
    local mask=$2
    printf "set /proc/irq/%d/smp_affinity to '%X'\n" $irq $mask
    printf "%X" $mask > /proc/irq/$irq/smp_affinity
}

#
# MAIN
#
DEV="eth4" # SET NIC
NUM_CPU=`grep processor /proc/cpuinfo | wc -l`

I=0
for IRQ in `cat /proc/interrupts | egrep -i "$DEV.*-[0-9]$" | cut -d: -f1 | sed "s/ //g"`
do
    MASK=$((1<<$I%$NUM_CPU)) # MASK = 1 << I % NUM_CPU
    I=$((I+1))
    set_affinity $IRQ $MASK
done
