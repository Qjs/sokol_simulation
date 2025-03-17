# source ~/emsdk/emsdk_env.sh
rm -rf build
mkdir build
cd build
emcmake cmake -DCMAKE_BUILD_TYPE=MinSizeRel ..
cmake --build . -- -j$(nproc)