# N dimensional Discrete Element Modelling code (N ⊂ ℕ*).
AKA Most Useless DEM.

This git repository contains the code for discrete element numerical modelling of granular material in a general number of spatial dimensions. The Dropbox folder for the code **should not** be used anymore.  

## Structure
The code is split into different modules, with different purposes. The main modules are:
- CppCode: `cpp` Main DEM simulation *@Franzzzzzzzz*
- visualise: `html+js` VR visualisation module of NDDEM *@benjym*
- CoarseGrainingCode: `cpp` Multi-dimensional coarse-graining code. Some parts are still triggered only for 3D.
- Liggghts: `lmp` Used for comparison of the NDDEM with existing DEM code in 3D, ie. Liggghts.
- PyCode: `py` Initial NDDEM code written in python, conserved for historical purposes only. Limited use and features.

## Support
Installation instructions and documentation is split between the c++ and javascript code, and can be found here:
 - [NDDEM solver and texturing server](https://franzzzzzzzz.github.io/NDDEM/CppCode/html/index.html)
 - [Visualisation](https://franzzzzzzzz.github.io/NDDEM/visualise/docs/index.html)
<!-- Additional information and project progress are available as a [Dropbox Paper](https://paper.dropbox.com/doc/N-Dimensional-DEM--ATnZ6ZOpm18JqKQGFYDe3eS0Ag-cM3nXtl2Yy4gNNfqlNYeS). -->


## Contributing
If you would like to contribute to the code, please contact francois.guillard [at] sydney.edu.au. For feature requests, please use the issue tracker above.

## License
This project is licensed under the GNU GPL License - see the LICENSE file for details

## Contact details
For help or questions, please contact francois.guillard [at] sydney.edu.au.

Edited 17/04/2020.
