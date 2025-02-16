/*#include <vector>
#include "Typedefs.h"
#include "ContactList.h"
#include "Cells.h"
#include "Octree.h"


#ifndef MAXDIM 
#define MAXDIM 4
#endif
#include "Preprocessor_macros.h"
#define MACRO(r, state) \
    case BOOST_PP_TUPLE_ELEM(2, 0, state): templatedmain<BOOST_PP_TUPLE_ELEM(2, 0, state)>(argv);break;



//BOOST_PP_FOR((1, MAXDIM), PRED, OP, MACRO)
template <int d >
 int init_cells (std::vector<Boundary<d>> &boundaries, std::vector<double> & r)
  {
    lvl_tree.resize(r.size(), -1) ; 
    
    auto [it_min, it_max] = std::minmax_element(r.begin(), r.end()) ; 
    double rmin = *it_min, rmax = *it_max ; 
    double ds = 2.1*rmax ; 
    
    char lvl = 0; 
    double smallestdim ; 
    
    do {
      octree.push_back(Cells<d>()) ; 
      
      if (lvl==0) octree[lvl].init_cells(boundaries, ds) ; 
      else octree[lvl].init_subcells(boundaries, octree[lvl-1].delta, octree[lvl-1].n_cell) ; 
      
      smallestdim = octree[lvl].delta[0] ;
      for (int dd=1 ; dd<d ; dd++) 
        if (octree[lvl].delta[dd]<smallestdim)
          smallestdim = octree[lvl].delta[dd] ; 
      
      lvl++ ; 
    }
    while (fit_in (r, smallestdim, lvl-1) > 0) ; 
    max_lvl = lvl ; 
    return 0;
  }

template int Octree<5>::init_cells (std::vector<Boundary<5>> &boundaries, std::vector<double> & r) ;
*/
