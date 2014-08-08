#!/bin/bash
TYPE="uncachable"
#TYPE="write-combining"
#TYPE="write-through"
#TYPE="write-protect"
#TYPE="write-back"
echo "disable=2" >| /proc/mtrr
echo "base=0xe0000000 size=0x20000000 type=$TYPE" >| /proc/mtrr
