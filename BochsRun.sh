#!/usr/bin/sh
make Image
cp Taurix.img ./BochsRun
cd BochsRun
bochs -f bochsrc.bxrc
