#! /bin/sh 

git clone --branch hdf5_1_8_18 https://bitbucket.hdfgroup.org/scm/hdffv/hdf5.git hdf5_1_8_18
cd hdf5_1_8_18
./configure --quiet --enable-shared --enable-production=yes --enable-debug=no --with-pic --disable-deprecated-symbols --disable-hl --disable-strict-format-checks --disable-clear-file-buffers --disable-instrument --disable-parallel --disable-trace --with-default-api-version=v18 CFLAGS="-O3 -w"
make install -C src
