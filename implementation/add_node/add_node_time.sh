#!/bin/bash

#sudo apt-get install gawk
#timeout 5s ./add_node_time.sh test

while true; do
	kubectl get nodes | awk '{if(NR==5) print strftime("%F %T") " " $1 " " $2}' >> $1.txt
done
