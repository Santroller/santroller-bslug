#!/bin/bash
make
cp bin/santroller-bslug.mod /run/media/sanjay/4A60-3302/bslug/modules
rm -rf /run/media/sanjay/4A60-3302/bslug/symbols
cp -rf symbols /run/media/sanjay/4A60-3302/bslug/symbols
# cp bin/wup-028-bslug.mod /run/user/1000/gvfs/ftp:host=192.168.0.126/sd/bslug/modules
sync
umount /run/media/sanjay/4A60-3302/