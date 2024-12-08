#!/bin/bash
make
rm -rf sd.zip
cp bin/santroller-bslug.mod bslug/modules
cp -rf symbols bslug
zip -r sd.zip bslug 
cp ../brainslug-wii/bin/boot.dol RGHE52.dol
zip sd.zip RGHE52.dol
rm RGHE52.dol