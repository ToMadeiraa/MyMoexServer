#!/bin/bash

#reading linux version
lsb_release -a > version_info 2> /dev/null
linux_distr=$(head -1 version_info | xargs echo | awk -v lin=3 '{print $ lin}')

if [ "$linux_distr" == "Ubuntu" ]; then
	echo "Found Ubuntu"

#install postgres
    sudo apt install postgresql postgresql-contrib

#creating database and tables
    sudo -u postgres createuser --createdb --login --pwprompt MoexUser
    sudo -u postgres createdb --owner=MoexUser moex

#install qt and dependencies
    sudo apt-get update && apt-get install -y \
    qt5-default \
    qtbase5-dev \
    qttools5-dev-tools \
    qtdeclarative5-dev \
    libqt5sql5-dev

#compile
    cd src
    qmake && make

#clear trash
    rm -rf moc_* *.o ../version_info .qmake.stash Makefile
    mv MyServer ../MyServer
    cd ..

#start server 
    ./MyServer

else
	echo "Not Ubuntu!!!"
fi


