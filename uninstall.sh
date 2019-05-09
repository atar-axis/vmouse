#!/bin/bash

if [[ $EUID != 0 ]]; then
  echo "ERROR: You most probably need superuser privileges to uninstall modules, please run me via sudo!"
  exit -3
fi


echo "* unloading current driver module"
modprobe -r vmouse

echo "  * uninstalling and removing v0.1 from DKMS"
dkms remove -m vmouse -v "0.1" --all

echo "  * removing $instance folder from /usr/src"
rm --recursive "/usr/src/vmouse-0.1/"
