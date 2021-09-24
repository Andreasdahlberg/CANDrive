#!/bin/sh

#Get source, use gitlab mirror as fallback.
if !(git clone https://git.cryptomilk.org/projects/cmocka.git) then
    echo "Clone from gitlab mirror..."
    if !(git clone https://gitlab.com/cmocka/cmocka.git) then
        exit 1
    fi
fi

#Compile and install
mkdir cmocka/build
cd cmocka/build
cmake -DCMAKE_INSTALL_PREFIX=/usr ..
make
make install

#Cleanup
cd ..
rm -rf cmocka
