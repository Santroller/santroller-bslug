#!/bin/bash
make
cp bin/santroller-bslug.mod /home/sanjay/.local/share/dolphin-emu/Load/WiiSDSync/bslug/modules/
rm -rf /home/sanjay/.local/share/dolphin-emu/Load/WiiSDSync/bslug/symbols
cp -rf symbols /home/sanjay/.local/share/dolphin-emu/Load/WiiSDSync/bslug
# cp bin/wup-028-bslug.mod /run/user/1000/gvfs/ftp:host=192.168.0.126/sd/bslug/modules
# sync
# umount /run/media/sanjay/4A60-3302/