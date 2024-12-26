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

### Goals:
Quick way to implement visualizations of both physical and mathematical concepts. 

Each Simulation will have: 
 - adjustable parameters
 - Plots over time
 - Renders of the concepts

### List of Visualizations to implement:
 - Pendulum
 - Monte Carlo Pi Estimation
 - Mass Spring Simulator
 - 2D Ising Model
 - Game of Life
 - Traveling Salesman
 - Mandlebrot set
 - Fluid Flow interaction
 - Heat Transfer
 - Hertzsprung-Russell Diagram
 - Psychrometric Chart
 - Schrodinger's Equation
 - Diffusion 2D random walks
