#! /bin/sh 

svn co -q https://svn.hdfgroup.org/hdf5/branches/hdf5_1_8_17@29907
cd hdf5_1_8_17
./configure --quiet --enable-shared --disable-production --enable-debug=all --with-pic --disable-deprecated-symbols --disable-hl --enable-strict-format-checks --enable-clear-file-buffers --enable-instrument --disable-parallel --enable-trace --with-default-api-version=v18 CFLAGS="-w"
make install -C src
