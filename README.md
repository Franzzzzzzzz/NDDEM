# N dimensional Discrete Element Modelling code (N ⊂ ℕ*).
AKA Most Useless DEM.

This git repository contains the code for discrete element numerical modelling of granular material in a general number of spatial dimensions. The Dropbox folder for the code **should not** be used anymore.  

Additional information and project progress are available as a [Dropbox Paper](https://paper.dropbox.com/doc/N-Dimensional-DEM--ATnZ6ZOpm18JqKQGFYDe3eS0Ag-cM3nXtl2Yy4gNNfqlNYeS).

The code is split in different modules, with different purposes. Some of the important folders are given there:
- CppCode: `cpp` Main DEM simulation *@franzzzzzzzz*
- visualise: `html+js` VR visualisation module of NDDEM *@benjy*
- CoarseGrainingCode: `cpp` Multi-dimensional coarse-graining code. Some parts are still triggered only for 3D.
- Liggghts: `lmp` Used for comparison of the NDDEM with existing DEM code in 3D, ie. Liggghts.
- PyCode: `py` Initial NDDEM code written in python, conserved for historical purposes only. Limited use and features.
- TestNetCDFjs: `js` Test for reading NetCDF files in javascript. Can probably be removed at some point


For help or questions, please contact francois.guillard [at] sydney.edu.au.

Edited 13/12/2018.
