# N dimensional Discrete Element Modelling code (N ⊂ ℕ*).
[For a live demonstration of NDDEM, see here.](https://franzzzzzzzz.github.io/NDDEM/visualise/docs/index.html#particle-visualisation)

This git repository contains the code for discrete element numerical modelling of granular material in a general number of spatial dimensions. The Dropbox folder for the code **should not** be used anymore.  

## Structure
The code is split into different modules, with different purposes. The main modules are:
- src/Dem: `cpp` Main DEM simulation *@Franzzzzzzzz*
- src/CoarseGraining: `cpp` Multi-dimensional coarse-graining code. *@Franzzzzzzzz*
- src/TexturingServer: `cpp` Server to generate textures to visualise rotating particles. *@Franzzzzzzzz*
- visualise: `html+js` VR visualisation module of NDDEM *@benjym*

## Operating system support
All parts of this code have been tested on all recent versions of Windows, Mac and Linux.

## Installation

### Ubuntu instructions

To install the required packages on a recent version of Ubuntu, try:

```
sudo apt install doxygen texlive texlive-font-utils cmake build-essential libboost-dev
```

### Installation of the NDDEM and coarse graining solvers without visualisation support:

From the root directory of this repository, make a `build` folder and move into it with:

```
mkdir build
cd build
```

Use `cmake` and then `make` on the `src` folder to compile the different components. The final binary executables will be stored within a newly created `bin` folder.

```
cmake ../src
make DEMND
cd ..
```

You can run an example from the `examples` folder as that oper:

```
./bin/DEMND 2 42 examples/in.example
```

To run a simulation on multiple threads, you need to run something like this in the terminal before running the simulation (replace 4 with the number of threads you want to use):

```
export OMP_NUM_THREADS=4
```

### Installation and test of the live visualisation. 
- Install and activate [emscripten](https://emscripten.org/docs/tools_reference/emsdk.html) first.
- Follow the steps in the script file CI_build_script.sh, adapting them to your setup.
- Then, in the `live` folder, you should be able to run the local visualisation with `webpack build && webpack serve`. 

## Documentation and examples
 - [NDDEM solver, coarse graining and texturing server](https://franzzzzzzzz.github.io/NDDEM/html/index.html)
 - [Visualisation](https://franzzzzzzzz.github.io/NDDEM/visualise/docs/index.html)
<!-- Additional information and project progress are available as a [Dropbox Paper](https://paper.dropbox.com/doc/N-Dimensional-DEM--ATnZ6ZOpm18JqKQGFYDe3eS0Ag-cM3nXtl2Yy4gNNfqlNYeS). -->

## Contributing
If you would like to contribute to the code, please contact francois.guillard [at] sydney.edu.au. For feature requests, please use the issue tracker above.

## License
This project uses many open source resources and is itself distributed under the GNU GPL License - see the INSTALL.md and LICENSE files for details.

## Contact details
For help or questions, please contact francois.guillard [at] sydney.edu.au.

<!--- Edited 27/04/2020. -->
