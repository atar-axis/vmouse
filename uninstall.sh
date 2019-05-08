#!/bin/bash

if [[ $EUID != 0 ]]; then
  echo "ERROR: You most probably need superuser privileges to uninstall modules, please run me via sudo!"
  exit -3
fi


echo "* unloading current driver module"
modprobe -r hid_pointeremu

echo "  * uninstalling and removing v0.1 from DKMS"
dkms remove -m hid-pointeremu -v "0.1" --all

echo "  * removing $instance folder from /usr/src"
rm --recursive "/usr/src/hid-pointeremu-0.1/"
