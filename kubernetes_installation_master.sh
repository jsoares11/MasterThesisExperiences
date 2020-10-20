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

sudo hostnamectl set-hostname master-node

sudo kubeadm init --pod-network-cidr=10.244.0.0/16

mkdir -p $HOME/.kube
sudo cp -i /etc/kubernetes/admin.conf $HOME/.kube/config
sudo chown $(id -u):$(id -g) $HOME/.kube/config

sudo kubectl apply -f https://raw.githubusercontent.com/coreos/flannel/master/Documentation/kube-flannel.yml