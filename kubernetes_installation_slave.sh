#!/bin/bash

sudo apt-get update

sudo apt -y install docker.io

sudo systemctl enable docker

sudo apt-get -y install curl

curl -s https://packages.cloud.google.com/apt/doc/apt-key.gpg | sudo apt-key add

sudo apt-add-repository "deb http://apt.kubernetes.io/ kubernetes-xenial main"

sudo apt -y install kubeadm

sudo swapoff -a

#sudo sed -i '15 s/^/#/' /etc/fstab
sudo sed -i '/ swap /s/^/#/' /etc/fstab

sudo hostnamectl set-hostname $1

#echo "kubeadm join $2 --token $3 $4 $5"
#sudo kubeadm join $2 --token $3 $4 $5