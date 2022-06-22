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

## Installation instructions and documentation
Installation instructions and documentation is split between the c++ and javascript code, and can be found here:
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
