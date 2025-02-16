
#ifndef OCTREE_H
#define OCTREE_H






template <int d>
class Octree {
public:
  std::vector<Cells<d>> octree ; 
  
  int init_cells (std::vector<Boundary<d>> &boundaries, std::vector<double> & r)
  {
    lvl_tree.resize(r.size(), -1) ; 
    
    auto [it_min, it_max] = std::minmax_element(r.begin(), r.end()) ; 
    double rmax = *it_max ; 
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
  //----------------------------------------------------------
  int cells_to_split()
  {
    return octree.back().cum_n_cell.back() ; 
  }
  //----------------------------------------------------------
  int fit_in (std::vector<double> &r , double smallestdim, char lvl)
  {
    int leftovers=0 ; 
    for (size_t i=0 ; i<lvl_tree.size() ; i++)
    {
      if (2*r[i]<smallestdim/2.) leftovers ++ ;       
      if (2*r[i]<smallestdim) lvl_tree[i] = lvl ; 
    }
    return leftovers ; 
  }
  //------------------------------------------------------------
  int allocate_to_cells (std::vector<std::vector<double>> & X)
  {
    for (int i=0 ; i<max_lvl ; i++)
      octree[i].clear_all() ; 
    
    //printf("A ") ; 
    for (size_t i=0 ; i<X.size() ; i++)
    {
      int res=octree[lvl_tree[i]].allocate_single(X[i], i) ;
      /*if (i==101 || i==103)
      {
        auto v = octree[lvl_tree[i]].id2x(res) ; 
        printf("%d, %d, %d %d %d ", lvl_tree[i], res, v[0], v[1], v[2]) ;  
        if (lvl_tree[i]==1) printf("->/2  %d ", octree[0].x2id({v[0]/2, v[1]/2, v[2]/2})) ;
      }*/
    }
    //printf("\n") ; 
    return 0;
  }
  //------------------------------------------------------------
  int contacts(std::pair<int,int> bounds, ContactList<d> & CLall, ContactList<d> & CLnew, std::vector<std::vector<double>> const & X, std::vector<double> const &r, double LE_displacement)
  {
    
    for (char lvl = max_lvl - 1 ; lvl>=0 ; lvl--)
    {
      octree[lvl].contacts(bounds, CLall, CLnew, X, r, LE_displacement) ; 
      
      for (char super_lvl=lvl-1 ; super_lvl>=0 ; super_lvl--)
        octree[lvl].contacts_external (octree[super_lvl], lvl-super_lvl, bounds, CLall, CLnew, X, r, LE_displacement) ; 
      
      std::get<0>(bounds)/= pow(2,d) ; 
      if (lvl>0 && std::get<1>(bounds) == octree[lvl].cum_n_cell[d]) std::get<1>(bounds)=octree[lvl-1].cum_n_cell[d] ;
      else std::get<1>(bounds) /= pow(2,d) ; 
    }    
    
    return 0 ; 
  }
  
  
  std::vector<char> lvl_tree ; 
  char max_lvl=-1 ;   
  
  
} ; 

#endif












