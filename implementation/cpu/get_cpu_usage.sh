#!/bin/bash

#sudo apt install sysstat

if [ "$1" != "" ]; then
    echo "Delay between cpu usage: " $1 
else
    usage
    echo ""
    echo "Please introduce the time elapsed between each cpu usage stat"
    echo "Example: ./get_cpu_usage 1 60 test -> will get the cpu usage every second 60 times"
    exit 1
fi

if [ "$2" != "" ]; then
    echo "Number of times to get cpu usage: " $2 
else
    usage
    echo ""
    echo "Please introduce the number of times to get cpu usage"
    echo "Example: ./get_cpu_usage 1 60 test -> will get the cpu usage every second 60 times"
    exit 1
fi

if [ "$3" != "" ]; then
    echo "Filename: " $3 
else
    usage
    echo ""
    echo "Please introduce the name of the file on which to save the cpu usage"
    exit 1
fi

mpstat $1 $2 | awk '{print $0}' >> $3.txt
#sed '$d' $3.txt
