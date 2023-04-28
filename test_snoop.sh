#!/bin/sh

set -x

# http_ip and http_port are for the http server hosted by the camera sensor
http_ip=192.168.0.137
http_port=80
storage_folder=test_dir

rm -rf $storage_folder
mkdir $storage_folder

if [ -f snoop ];
then
    echo "Starting snoop"
    ./snoop ./$storage_folder $http_ip $http_port
fi