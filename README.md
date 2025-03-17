# WASM Sokol CIMGUI CIMPLOT Simulation Playground

This project uses CIMGUI and CIMPLOT to plot interactive simulations by selection.
I wanted to make a framework to very steadily add cool vislulations/simulations as I am able to.

I landed on using an Xmacro list of the simulations with their specific functions for each render/control making it scalable and easy to template against.


Prerequisites:
- Emscripten SDK
- CMake

How to build
```
git clone https://github.com/qjs/sokol_simulation.git
git submodule update --init --recursive

# Where ever you store the emsdk
source ~/emsdk/emsdk_env.sh
./build.sh
```

Current Simulations:
- None
- Pendulum
