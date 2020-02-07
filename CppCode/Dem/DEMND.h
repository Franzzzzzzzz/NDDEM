#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <vector>
#include <ctime>
#include <cstring>
#include <omp.h>

#include "Typedefs.h"
#include "Parameters.h"
#include "Contacts.h"
#include "ContactList.h"
#include "Multiproc.h"


/** \mainpage
\section Gen General description
NDDEM is a full suit of tools to perform simulations and visualisions of granular media in any spatial dimension. This documentation gives information on the C++ code base, which has 3 modules. **Dem** is the N-dimensional simulation module, **CoarseGraining** performs coarse-graining of the data from the simulations, **Texturing** handles the rendering as UV maps of the orientation of the hyperspheres, and is designed to be used as an http server in conjunction with the Threejs visualisation module.

\section Prog Program structure

\dot
digraph A {
Input -> Dem ;
Dem -> Results;
Results -> CoarseGraining;
Input -> CoarseGraining;
CoarseGraining -> CGData;
CGData -> Visualisation;
Results -> Texturing;
Texturing -> Textures;
Textures -> Visualisation;
Input -> Visualisation;
Results -> Visualisation;
Input [label="Input File"];
CGData [label="Coarse-grained data"] ;
Dem [shape=box,style=filled,color=".8 1.0 .8"] ;
CoarseGraining [shape=box,style=filled,color=".8 1.0 .8"] ;
Texturing [shape=box,style=filled,color=".6 1.0 .6"] ;
Visualisation [shape=box,style=filled,color="0.9 0.6 0.6"] ;
}
\enddot

*/
