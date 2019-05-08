#!/bin/bash

# exit immediately if one command fails
set -e

if [[ $EUID != 0 ]]; then
  echo "ERROR: You most probably need superuser privileges to install new modules, please run me via sudo!"
  exit -3
fi


echo "* copying module into /usr/src"
cp --recursive "$PWD/vmouse/" "/usr/src/vmouse-0.1"

echo "* adding module to DKMS"
dkms add -m vmouse -v "0.1"

echo "* installing module (using DKMS)"
dkms install -m vmouse -v "0.1"
