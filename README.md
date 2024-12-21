# Sokol Simulations


### Prequisites:
Emscripten SDK
https://emscripten.org/docs/getting_started/downloads.html


### How to build
From the repo:
```
git pull
git submodule update --init --recursive
mkdir ./build/
cd ./build/
emcmake cmake -DCMAKE_BUILD_TYPE=M
inSizeRel -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
cmake --build .
```

### How to run
From build dir
```
python3 -m http.server
```

