# README
ARGoS3-CMAES-XnES
=====================


## Description

The code maintained on this repo allows to use xNES and CMA-ES to optimise a neural network control software for the [ARGoS3 simulator](https://github.com/ilpincy/argos3).
The optimization algorithm are adaptations of the [pagmo2 library](https://github.com/esa/pagmo2). Multi-threading using [open MPI](https://github.com/open-mpi/ompi) has been implemented for CMA-ES and xNES for cluster use.


## Package content

- `bin` This empty folder will contain the executable files.
- `params` This directory contains parameters used for the CMA-ES and xNES based design methods
- `src` This directory contains the source code for the design methods implemented in this repository
    - `genome_parser` visualizer for single layer Neural Network control software.
    - `NEAT` This directory contains the library used for the neural network topology.
    - `pagmo` The modified pagmo2 library.
    - `program` This directory contains the source code of the executables.
    - `NEATController.cpp` The definition of the neural network control software.
- `startgen` This directory contains the initial genomes for the design methods

## Installation
### Dependencies:
- [ARGoS3](https://github.com/ilpincy/argos3) (3.0.0-beta48)
- [argos3-epuck](https://github.com/demiurge-project/argos3-epuck) (v48)
- [demiurge-epuck-dao](https://github.com/demiurge-project/demiurge-epuck-dao) (master)
- [NEAT](https://github.com/demiurge-project/ARGoS3-NEAT)
- [pagmo2](https://github.com/esa/pagmo2) (master)
- [Eigen3](http://eigen.tuxfamily.org/index.php?title=Main_Page) (v3.3+)
- [boost](https://www.boost.org/) (1.60+)
- [MPI](https://www.open-mpi.org/) (for parallelization)

A compiler with C++17 support (e.g. GCC > 7)

### Compiling ARGoS3-CMAES-XnES:
    $ git clone https://github.com/demiurge-project/ARGoS3-pagmo2
    $ cd ARGoS3-pagmo2
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make

Once compiled, the `bin/` folder should contain the `NEAT-evolution`, `scheduler` and `NEAT-launch`
executable.

## How to use
### Run a single experiment:
```bash
./bin/NEAT-launch -c mission.argos -g genome_to_test
```
### Run the design process
# Command to launch
1. To launch the Evolutionary Process which uses NEAT and ARGoS (with the epuck robot):
* Parallel
```bash
./bin/NEAT-evolution -g startgen/mlp_choco.ge -m 4 -b bin/scheduler -p params/xn_s0.5_p100.pa -c mission.argos
```
where `startgen/mlp_choco.ge` is the stater genome file, which contains the definition of the 1st genome.
where `-m 4` is the nb of processes
where `-p params/xn_s0.5_p100.pa` is the parameter file for CMA-ES or XNES
where `-c mission.argos` is the mission file

# Create your own experiment

- If you want to create a new experiment with the current epuck's controller, which uses 8 proximity sensors, 8 light sensors, 3 ground sensors, 3 range-and-bearing sensors, a bias unit as inputs and 2 wheel actuators as outputs, you just need to create a new loop-function (which will evaluate the neural network), and possibly a new argos configuration file. Apart from those 2, You don’t need to create/change anything else.

- If instead you want to use another robot or the epuck with a different set of inputs/outputs, you will need to create your own controller, starter genome, and a new argos configuration file, in addition to the loop-function (if you want to create your own experiment).

- If you want to use just NEAT without the simulator ARGoS. You will need to modify the main program: in the main method, you will need to initialize your own experiment, then call the method launchNEAT(…) by passing your own defined method as a parameter.
launchNEAT(…) is a method that expects at least 3 arguments: the neat parameters file, the starter genome and your function that launches your experiment and evaluates an organism/network or population on this one. This method will set the evolutionary process and will call your method in which you are supposed to launch your experiment and evaluate the organisms/networks. After calling your method, launchNEAT(…) will evolve the population for the next generation.

Your method should accept only one parameter NEAT::Network& or NEAT::Population&.
-> If your method has NEAT::Network& as a parameter: you should, after launching the experiment and evaluating this network on it, return the fitness value.
-> If your method has NEAT::Population& as a parameter: you should launch the experiment for each organism and evaluate each one of them. Your method doesn’t need to return anything.

