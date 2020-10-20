#!/bin/bash

#sudo apt-get install gawk
#timeout 5s ./scale_time.sh test

while true; do
	#sudo kubectl get replicasets -n kafka | awk '{if(NR==3) print strftime("%F %T") " " $0}' >> $1.txt
	kubectl get replicasets -n kafka | awk '{if(NR==3) print strftime("%F %T") " " $0}' >> $1.txt
done
