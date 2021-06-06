#!/bin/sh

#Get source
git clone https://git.cryptomilk.org/projects/cmocka.git

#Compile and install
mkdir cmocka/build
cd cmocka/build
cmake -DCMAKE_INSTALL_PREFIX=/usr ..
make
make install

#Cleanup
cd ..
rm -rf cmocka
