#!/bin/bash
echo $whoami
if ((EUID != 0))
	then echo "Must be run as root"
	exit
fi

echo "Installing Things Used for ECE498 Testing"
apt-get update
apt-get upgrade
apt-get install libasound2 alsa-utils alsa-oss
apt-get install git
apt-get install festival
apt-get install festival-dev

echo "Installing Wire-GPIO"
git clone https://github.com/friendlyarm/WiringNP
cd WiringNP
chmod 755 build
./build

gpio readall



