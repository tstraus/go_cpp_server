export CC=/usr/bin/clang
export CXX=/usr/bin/clang++

mkdir build
cd build

cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
