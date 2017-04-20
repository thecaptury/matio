#! /bin/sh 

curl https://support.hdfgroup.org/ftp/lib-external/szip/2.1.1/src/szip-2.1.1.tar.gz -O -J
tar -zxf szip-2.1.1.tar.gz
cd szip-2.1.1
./configure --quiet --enable-static=no CFLAGS="-w"
make install -C src
cd ..
git clone --quiet --depth 1 --branch hdf5_1_8_18 https://bitbucket.hdfgroup.org/scm/hdffv/hdf5.git hdf5_1_8_18
cd hdf5_1_8_18
./configure --enable-shared --disable-production --enable-debug=all --with-pic --disable-deprecated-symbols --disable-hl --disable-strict-format-checks --disable-clear-file-buffers --disable-instrument --disable-parallel --disable-trace --with-default-api-version=v18 --with-szlib=$TRAVIS_BUILD_DIR/szip-2.1.1/szip CFLAGS="-w"
make install -C src
