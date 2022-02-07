# Compilation (all platforms)
- Navigate to the build folder at the root of the repository
- In this folder, execute the following commands in order:
`cmake ../src`
`make`
- Once the compilation is successful, a `bin` folder should have been created at the root of the repository, containing 3 executables: `DEMND` to perform N-Dimensional simulations, `TexturingServer` to handle the visualisation, `CoarseGraining` to perform the data coarse-graining.

Windows users: on Windows, the program can be successfully compiled using Cygwin https://www.cygwin.com/.

# Example
## Simulation
The `examples` folder contains some script to run the simulations. You may run a simulation from the `examples` directory by running:
`../bin/DEMND 5 1445 in.inclinedD5`
where the two numbers are taken from the `dimensions` line in the corresponding input script file (in this example, `in.inclinedD5`.

## Visualistion
To run a visualisation, you first need to start the server in the bin folder: `TexturingServer`.
Once the server is running, navigate using your favorite browser (only Firefox and Chrome compatible) to the `index.html` file in the `visualisation` at the root of the repository.


# Prerequisites
- Cmake: tested with version 3.10.2. https://cmake.org/
- Boost: tested with version 1.66.0. https://www.boost.org/
- C++ compiler with openmp support. Tested with gcc compiler. Minimum version 7.4. https://gcc.gnu.org/

# Optional prerequisites
- Doxygen: to compile the documentation. Tested with version 1.8.13. http://www.doxygen.nl/
- Matlab: to save the coarse-graining result in a mat file. If cmake does not find your Matlab instalation, you may need to define the matlab path: `export Matlab_ROOT_DIR=/path/to/Matlab`

# External Libraries and resources included in the sources
- NrrdIO-1.11.0: Distributed under LGPLv2. Consult the file src/NrrdIO-1.11.0-src/LICENSE_lgpl-2.1.txt for more information, or the project website: http://teem.sourceforge.net/nrrd/index.html
- gzip: Distributed under the Boost Software License, Version 1.0.
- zlib: Distributed under the Boost Software License, Version 1.0.
- httplib.h: Distributed under MIT License. https://github.com/yhirose/cpp-httplib
- Tinypng: Distributed under LGPLv3. https://www.nayuki.io/page/tiny-png-output
- Threejs: Distributed under MIT licence. https://threejs.org/
- papaparse: Distributed under MIT licence. https://www.papaparse.com/
- aframe: Distributed under MIT licence. https://github.com/aframevr/aframe
- 1227 Earth.bin: Poly by Google. Distributed under CC-BY. https://poly.google.com/view/58PjkXNdpPb
- eso0932a.jpg: ESO. Distributed under CC-BY. https://www.eso.org/public/images/eso0932a/
