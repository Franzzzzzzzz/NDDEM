#ifndef CELLCONTACTDETECTION
#define CELLCONTACTDETECTION

/** \addtogroup DEM
 *  @{ */
#include <boost/random.hpp>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <chrono>
#include <algorithm>
#include <numeric>

#include "Boundaries.h"
#include "ternary.h"

/** \brief Individual cell for contact detection
 */
class Cell {
public: 
  std::vector<int> incell ; ///< list of particles in the cell
  std::vector<int> neighbours ;  ///< list of cell neighbours to the current cell
  std::vector<int> neigh_ghosts ; ///< list of the ghost cells through the PBC
  std::vector<bitdim> neigh_ghosts_dim ; ///< Ghost cell dimensions going through PBC
  std::vector<bitdim> neigh_ghosts_dir ; ///< Ghost cell directions going through PBC (1 is negative delta)
  
  std::vector<int> neighbours_full ;  ///< list of cell neighbours to the current cell
  std::vector<int> neigh_ghosts_full ; ///< list of the ghost cells through the PBC
  std::vector<bitdim> neigh_ghosts_full_dim ; ///< Ghost cell dimensions going through PBC
  std::vector<bitdim> neigh_ghosts_full_dir ; ///< Ghost cell directions going through PBC (1 is negative delta)
  
  template <class Archive>
  void serialize( Archive & ar )
  {
    ar(incell, neighbours, neigh_ghosts, neigh_ghosts_dim, neigh_ghosts_dir, neighbours_full, neigh_ghosts_full, neigh_ghosts_full_dim, neigh_ghosts_full_dir) ; 
  }
  
} ;

/** \brief All the cells making the space, with related function for creating the cell array, neighbour arrays etc. 
 * Currently non implemented TODO: no pbc handling, no handling of moving boundaries. 
 */
template <int d>
class Cells {
public: 
  int init_cells (std::vector<Boundary<d>> &boundaries, double ds) ; 
  int init_cells (std::vector<Boundary<d>> &boundaries, std::vector<double> & r) {double ds = 2.1*(*std::max_element(r.begin(), r.end())) ; return (init_cells(boundaries, ds)) ; }
  int init_subcells (std::vector<Boundary<d>> &bounds, double delta_super[], const std::vector<int> & n_cell_super) ;
  int init_allocate() ;
  int init_finalise(std::vector<Boundary<d>> &bounds) ; 
  std::vector<std::vector<double>> boundary_handler (std::vector<Boundary<d>> &boundaries) ;
  int allocate_to_cells (std::vector<std::vector<double>> & X) ; 
  int allocate_single (std::vector<double> & X, int grainid) ; 
  int contacts (int ID, std::pair<int,int> bounds, CLp_it_t<d> & CLp_it,
                ContactList<d> & CLnew, std::vector<std::vector<double>> const & X, std::vector<double> const &r, double LE_displacement) ; 
  int contacts_external (int ID, Cells<d> &ocell, int delta_lvl, std::pair<int,int> bounds, CLp_it_t<d> & CLp_it,
                         ContactList<d> & CLnew, std::vector<std::vector<double>> const & X, std::vector<double> const &r, double LE_displacement) ;
  int contacts_cell_cell (int ID, Cell & c1, Cell & c2, CLp_it_t<d> & CLp_it, 
                          ContactList<d> & CLnew,std::vector<std::vector<double>> const & X, 
                          std::vector<double> const &r, cp<d> & tmpcp ) ; 
  int contacts_cell_ghostcell (int ID, Cell & c1, Cell & c2, bitdim ghost, bitdim ghostdir, CLp_it_t<d> & CLp_it, 
                               ContactList<d> & CLnew, std::vector<std::vector<double>> const & X, std::vector<double> const &r, double LE_displacement, cp<d> & tmpcp ) ;
  bool cell_contact_insert (int ID, CLp_it_t<d> & CLp_it, const cp<d>& a) ; 
  int clear_all () ; 
  
  //-------------------------------------------
  int x2id (const std::vector<int>&v) 
  {
    int res=0 ; 
    for (int dd=0; dd<d ; dd++)
    {
      if (v[dd] < 0 || v[dd]>=n_cell[dd]) {/*printf("WARN: particle outside the cell volume\n") ;*/ fflush(stdout) ; return -1 ; }
      res += cum_n_cell[dd] * v[dd] ; 
    }
    return res ; 
  }
  //-------------------------------------------
  std::vector<int> id2x (int id)  
  {
    std::vector<int> ids (d,0) ;
    for (int dd=0 ; dd<d ; dd++)
    {
      ids[dd] = id % n_cell[dd] ; 
      id /= n_cell[dd] ; 
    }    
    return ids ;
  }
  //-------------------------------------------
  bool test_idempotent ()
  {
    for (int i=0 ; i<cum_n_cell[d] ; i++)
    {
      if (x2id(id2x(i))!=i)
      {
        printf("ERROR %d\n", i) ; 
        return false ; 
      }
    }
    return true ;         
  }
  //---------------------------------------------
  double dsqr (std::vector<double> &X1, std::vector<double> &X2)
  {
    double sum = 0 ; 
    for (int k=0 ; k<d ; k++)
      sum += (X1[k]-X2[k])*(X1[k]-X2[k]) ; 
    return sum ; 
  }
  //---------------------------------------------
  int clip_LEmoved_cell(int original, int considered, bitdim &ghost, bitdim &ghostdir)
  {
    int offset = (original/planesize)*planesize ;
    
    if (considered < offset)
    {
      considered += planesize ; 
      if ((ghost>>1)&1 && (ghostdir&1))
        ghost &= ~(1<<1) ;
      else
      {
        ghost |= (1<<1) ;
        ghostdir |= (1<<1) ; 
      }
    }
    
    if (considered >= offset+planesize)
    {
      considered -= planesize ; 
      if ((ghost>>1)&1 && !(ghostdir&1))
        ghost &= ~(1<<1) ;
      else
        ghost |= (1<<1) ; 
    }
    
    return considered;           
  }
  
  std::vector<int> n_cell , cum_n_cell ; 
  std::vector<double> origin ; 
  std::vector<double> Delta ; 
  std::vector<Cell> cells ;
  double delta[d] ;
  int planesize=0 ;
    
  template <class Archive>
    void serialize( Archive & ar )
    {
      ar(n_cell, cum_n_cell, CEREAL_NVP(origin), CEREAL_NVP(Delta), cells, delta, planesize) ; 
    }
} ; 

/** @}*/

/*****************************************************************************************************
 *                                                                                                   *
 *                                                                                                   *
 *                                                                                                   *
 * IMPLEMENTATIONS                                                                                   *
 *                                                                                                   *
 *                                                                                                   *
 *                                                                                                   *
 * ***************************************************************************************************/

template <int d>
std::vector<std::vector<double>> Cells<d>::boundary_handler (std::vector<Boundary<d>> &boundaries)
{
  std::vector<std::vector<double>> res (d, std::vector<double>(2,0)) ;

  for (size_t i=0 ; i<boundaries.size() ; i++)
  {
    switch (boundaries[i].Type)
    {
      case WallType::PBC:
      case WallType::PBC_LE:
                res[i][0] = boundaries[i].xmin ;
                res[i][1] = boundaries[i].xmax ;
                Delta.resize(d) ; 
                Delta[i] = boundaries[i].delta ; 
                break ; 
      case WallType::MOVINGWALL:
      case WallType::ELLIPSE:
                printf("ERR: this type of boundary is incompatible with cell contacts\n") ;
                break ;
      case WallType::WALL:
                res[i][0] = boundaries[i].xmin ;
                res[i][1] = boundaries[i].xmax ;
                break ;
      case WallType::SPHERE:
      case WallType::ROTATINGSPHERE:
                   for (int dd=0 ; dd<d ; dd++)
                   {
                      res[dd][0] = boundaries[i].center[dd]-boundaries[i].radius ;
                      res[dd][1] = boundaries[i].center[dd]+boundaries[i].radius ;
                   }
                   break ;
      case WallType::HEMISPHERE:
                   for (int dd=0 ; dd<d ; dd++)
                   {
                      res[dd][0] = boundaries[i].center[dd]-boundaries[i].radius ;
                      if (dd != boundaries[i].axis)
                        res[dd][1] = boundaries[i].center[dd]+boundaries[i].radius ;
                   }
                   break ;
      case WallType::AXIALCYLINDER:
                   for (int dd=0 ; dd<d ; dd++)
                   {
                     if (dd==boundaries[i].axis) continue ;
                     res[dd][0] = boundaries[i].radius ;
                     res[dd][1] = boundaries[i].radius ;
                   }
                   break ;
      default:
        printf("ERR: unknown boundary condition for cell building.\n") ;
        break;
    }
  }
  return res ;
}

//------------------------------------------------------------------------------------
template <int d>
int Cells<d>::init_subcells (std::vector<Boundary<d>> &bounds, double delta_super[], const std::vector<int> & n_cell_super)
{
  init_allocate() ; 
  for (int i=0 ; i<d ; i++)
  {
    n_cell[i] = n_cell_super[i]*2 ; 
    delta[i] = delta_super[i]/2. ; 
  }
  
  init_finalise(bounds) ;   
  return 0; 
}
//-------------------------------------------
template <int d>
int Cells<d>::init_cells(std::vector<Boundary<d>> &bounds, double ds)
{
  printf("CELL SIZE: %g\n", ds) ; 

  init_allocate() ; 
  
  auto boundaries = boundary_handler(bounds) ;
  for (int i=0 ; i<d ; i++)
  {
    n_cell[i] = floor((boundaries[i][1]-boundaries[i][0])/ds) ; 
    delta[i] = (boundaries[i][1]-boundaries[i][0])/n_cell[i] ; 
  }
  
  init_finalise(bounds) ; 
  return 0 ; 
}  
//-------------------------------------------
template <int d>
int Cells<d>:: init_allocate ()
{
  n_cell.resize(d,0);
  cum_n_cell.resize(d+1,0) ; 
  origin.resize(d,0) ;
  return 0; 
}
//-------------------------------------------
template <int d>
int Cells<d>::init_finalise(std::vector<Boundary<d>> &bounds)
{
  auto boundaries = boundary_handler(bounds) ;
  cum_n_cell[0] = 1 ; 
  for (int i=0 ; i<d ; i++)
  {
    if (i>0) 
      cum_n_cell[i] = cum_n_cell[i-1]*n_cell[i-1] ; 
    origin[i] = boundaries[i][0] ; 
  }
  cum_n_cell[d] = cum_n_cell[d-1]*n_cell[d-1] ; 
  planesize = cum_n_cell[2] ; 
  
  cells.resize(cum_n_cell[d]) ; 
  for (auto &v: n_cell) printf("%d ", v) ;
  printf("\n") ; 
  for (auto &v: cum_n_cell) printf("%d ", v) ; 
  printf("\n") ;   
  for (int i=0 ; i<d ; i++)
    printf("%g ", delta[i]) ; 
  printf("\n") ; 
  
  for (size_t i=0 ; i<cells.size() ; i++)
  {
    auto x_base = id2x(i) ; 
    auto x = x_base ; 
    //for (int dd=0 ; dd<d ; dd++) tmp[dd] = x_base[dd] ; 
    
    //auto res = hilbert::v2::PositionToIndex<uint8_t, d> (tmp) ; 
    //for (auto & v: res) 
    //  printf("%d ", v) ; 
    //printf("\n") ; 
    //cells[i].hilbert_idx = hilbert::v2::PositionToIndex<int, d> (tmp) ; 
    
    cells[i].neighbours.reserve(pow(3,d)) ; 
    
    ternary t ; 
    if (bounds[0].Type==WallType::PBC_LE) t.set_quat_bit(1) ; 
    t++ ; 
    
    for ( ; t<pow(3,d) ; t++)
    {
      bool noadd = false ;
      int pbc_le_crossing = 0; 
      x = x_base ; 
      
      bitdim ghost = 0, ghostdir = 0 ; 
      for (int dd=0 ; dd<d ; dd++)
      {
        if ( t[dd]==2 )
        {
          if (pbc_le_crossing != 0 )
            x[dd] += t[dd] * pbc_le_crossing ; 
          else
            noadd = true ; 
        }
        else
          x[dd] += t[dd] ; 
        
        if (x[dd]<0 ) 
        {
          if (bounds[0].Type==WallType::PBC_LE && dd==0)
          {
            pbc_le_crossing = 1 ; 
            ghost |= 1<<dd ; 
            ghostdir |= 1<<dd ;
            x[dd] += n_cell[dd] ;             
          }
          else if (bounds[dd].Type==WallType::PBC) // Only using positive ghosts
          {
            ghost |= 1<<dd ; 
            ghostdir |= 1<<dd ;
            x[dd] += n_cell[dd] ; 
          }
          else
          {
            noadd=true ; 
            break ; 
          }
        }
        else if (x[dd]>=n_cell[dd])
        {   
          if (bounds[0].Type==WallType::PBC_LE && dd==0)
          {
            pbc_le_crossing = -1 ; 
            ghost |= 1<<dd ; 
            x[dd] -= n_cell[dd] ; 
          }           
          else if (bounds[dd].Type==WallType::PBC) // Only using positive ghosts
          {
            ghost |= 1<<dd ; 
            x[dd] -= n_cell[dd] ; 
          }
          else
          {
            noadd = true ; 
            break ; 
          }
        }      
      }
      if (!noadd)
      {
        if (ghost>0)
        {
          unsigned int id = x2id(x) ; 
          if (bounds[0].Type==WallType::PBC_LE && (ghost&1)) // Add all the ghosts crossing the LE pbc positively
          {
            if (!(ghostdir&1))
            {
              cells[i].neigh_ghosts.push_back(id) ; 
              cells[i].neigh_ghosts_dim.push_back(ghost) ; 
              cells[i].neigh_ghosts_dir.push_back(ghostdir) ; 
            }
          }         
          else 
          {
            if (id>i) // if not a ghost pbc, just add. 
            {
              cells[i].neigh_ghosts.push_back(id) ; 
              cells[i].neigh_ghosts_dim.push_back(ghost) ; 
              cells[i].neigh_ghosts_dir.push_back(ghostdir) ; 
            }
            else 
            {
              cells[i].neigh_ghosts_full.push_back(id) ; 
              cells[i].neigh_ghosts_full_dim.push_back(ghost) ; 
              cells[i].neigh_ghosts_full_dir.push_back(ghostdir) ; 
            }
          }
        }
        else
          cells[i].neighbours.push_back(x2id(x)) ;
      }
    }
    
    sort( cells[i].neighbours.begin(), cells[i].neighbours.end() );
    int bef = cells[i].neighbours.size() ; 
    cells[i].neighbours.erase( unique( cells[i].neighbours.begin(), cells[i].neighbours.end() ), cells[i].neighbours.end() );
    int aft = cells[i].neighbours.size() ; 
    if (bef-aft != 0) printf("ERR: no duplication of cells should happen with the algorithm.\n") ;  
    
    cells[i].neighbours_full = cells[i].neighbours ;     
    cells[i].neighbours.erase(std::remove_if(cells[i].neighbours.begin(), cells[i].neighbours.end(), 
                                            [=](size_t x) { return x<=i ; }), cells[i].neighbours.end());
    cells[i].neighbours_full.erase(std::remove_if(cells[i].neighbours_full.begin(), cells[i].neighbours_full.end(), 
                                            [=](size_t x) { return x>=i ; }), cells[i].neighbours_full.end());
  }
    
  /*for (int i=0 ; i<cum_n_cell[d] ; i++)
  {
    printf("%d %ld | ", i, cells[i].neighbours.size()) ;    
    for (size_t j=0 ; j< cells[i].neighbours.size() ; j++)
      printf("%d ", cells[i].neighbours[j]) ;   
    printf("|") ; 
    for (size_t j=0 ; j< cells[i].neighbours_full.size() ; j++)
      printf("%d ", cells[i].neighbours_full [j]) ; 
    printf("||") ; 
    for (size_t j=0 ; j< cells[i].neigh_ghosts.size() ; j++)
      printf("%d ", cells[i].neigh_ghosts [j]) ; 
    printf("|") ; 
    for (size_t j=0 ; j< cells[i].neigh_ghosts_full.size() ; j++)
      printf("%d ", cells[i].neigh_ghosts_full [j]) ; 
    printf("\n") ; 
  }*/
  
  return 0 ; 
}
//=================================================================
template <int d>
int Cells<d>::allocate_to_cells (std::vector<std::vector<double>> & X)
{
  std::vector<int> v (d,0) ; 
  
  clear_all() ; 
  
  for (size_t i=0 ; i<X.size() ; i++)
  {
    for (int dd=0 ; dd<d ; dd++)
      v[dd] = floor((X[i][dd]-origin[dd])/delta[dd]) ; 
    int id = x2id(v) ; 
    if (id != -1)
      cells[id].incell.push_back(i) ;
  }
  
  /*int tot = 0 ;
  for (size_t i=0 ; i<cells.size() ; i++)
    tot += cells[i].incell.size() ;
  printf("%d ", tot) ; fflush(stdout) ;*/
  return 0 ; 
}
//=================================================================
template <int d>
int Cells<d>::clear_all ()
{
  //printf("==> %ld ", cells.size()) ; fflush(stdout) ; 
  for (size_t i=0 ; i<cells.size() ; i++) 
    cells[i].incell.clear() ; 
  return 0 ;
}
//=================================================================
template <int d>
int Cells<d>::allocate_single (std::vector<double> & X, int grainid)
{
  std::vector<int> v (d,0) ; 
  
  for (int dd=0 ; dd<d ; dd++)
    v[dd] = floor((X[dd]-origin[dd])/delta[dd]) ; 
  int id = x2id(v) ; 
  if (id != -1)
    cells[id].incell.push_back(grainid) ;
  
  /*int tot = 0 ;
  for (size_t i=0 ; i<cells.size() ; i++)
    tot += cells[i].incell.size() ;
  printf("%d ", tot) ; fflush(stdout) ;*/
  return id ; 
}
//=================================================================
template <int d>
int Cells<d>::contacts (int ID, std::pair<int,int> bounds, CLp_it_t<d> & CLp_it,
                        ContactList<d> & CLnew, 
                        std::vector<std::vector<double>> const & X, std::vector<double> const &r, double LE_displacement)
{
  auto [cell_first, cell_last] = bounds ; 
  cp<d> tmpcp(0,0,0,nullptr) ; tmpcp.persisting = true ; 
  double sum=0 ;
  int ncontact=0 ; 
  //printf("B%d ", ID) ; fflush(stdout) ; 
  for (int c=cell_first ; c<cell_last ; c++)
  {
    if (cells[c].incell.size()==0) continue ; 
    // Inner cell contact
    for (size_t ii=0 ; ii<cells[c].incell.size() ; ii++)
      for (size_t jj=ii+1 ; jj<cells[c].incell.size() ; jj++)
      {
        sum = 0 ; 
        int i = cells[c].incell[ii] ;
        int j = cells[c].incell[jj] ;
        for (int k=0 ; sum<(r[i]+r[j])*(r[i]+r[j]) && k<d ; k++) 
          sum+= (X[i][k]-X[j][k])*(X[i][k]-X[j][k]) ;
        if (sum<(r[i]+r[j])*(r[i]+r[j]))
        {
            ncontact++ ;
            if (i<j) {tmpcp.i = i ; tmpcp.j=j ;}
            else { tmpcp.i = j ; tmpcp.j=i ;} 
            tmpcp.contactlength=sqrt(sum) ; tmpcp.ghost=0 ; tmpcp.ghostdir=0 ;
            if (!cell_contact_insert(ID, CLp_it, tmpcp)) 
            {
              CLnew.v.push_back(tmpcp) ;
            }
        }
      }
    
    //neighbours contacts
    for (size_t cc=0 ; cc<cells[c].neighbours.size() ; cc++)
    {
      int c2 = cells[c].neighbours[cc] ; 
      for (size_t ii=0 ; ii<cells[c].incell.size() ; ii++)
        for (size_t jj=0 ; jj<cells[c2].incell.size() ; jj++)
        {
          sum = 0 ; 
          int i = cells[c].incell[ii] ;
          int j = cells[c2].incell[jj] ;
          for (int k=0 ; sum<(r[i]+r[j])*(r[i]+r[j]) && k<d ; k++) 
            sum+= (X[i][k]-X[j][k])*(X[i][k]-X[j][k]) ;
          if (sum<(r[i]+r[j])*(r[i]+r[j]))
          {
            ncontact++ ; 
            if (i<j) {tmpcp.i = i ; tmpcp.j=j ;}
            else { tmpcp.i = j ; tmpcp.j=i ;} 
            tmpcp.contactlength=sqrt(sum) ; tmpcp.ghost=0 ; tmpcp.ghostdir=0 ;
            if (!cell_contact_insert(ID, CLp_it, tmpcp)) CLnew.v.push_back(tmpcp) ; 
          }
      }    
    }
    
    //ghost contacts
    //printf("[%d ", c) ; 
    for (size_t cc=0 ; cc<cells[c].neigh_ghosts.size() ; cc++)
    {
      int c2 = cells[c].neigh_ghosts[cc] ; 
      bitdim ghost=cells[c].neigh_ghosts_dim[cc], ghostdir=cells[c].neigh_ghosts_dir[cc] ; 
      
      if (LE_displacement && cells[c].neigh_ghosts_dim[cc]&1) // This is a ghost cell through a lees-edward BC
      {
        int newc2 = c2-floor(LE_displacement*(cells[c].neigh_ghosts_dir[cc]&1?-1:1)/delta[1]) * cum_n_cell[1] ;
        c2 = clip_LEmoved_cell(c2, newc2, ghost, ghostdir) ;
      }
      
      //printf("%d ", c2) ; 
      
           
      for (size_t ii=0 ; ii<cells[c].incell.size() ; ii++)
        for (size_t jj=0 ; jj<cells[c2].incell.size() ; jj++)
        {
          double sum = 0 ; 
          int i = cells[c].incell[ii] ;
          int j = cells[c2].incell[jj] ;
          for (int k=0 ; sum<(r[i]+r[j])*(r[i]+r[j]) && k<d ; k++) 
          {
            double additionaldelta=0 ; 
            if (k==1 && ghost&1)
              additionaldelta = LE_displacement * (ghostdir&1?-1:1) ;
            sum+= pow(( X[i][k]-X[j][k] - Delta[k]*(ghost>>k & 1)*((ghostdir>>k & 1)?-1:1) - additionaldelta),2) ;
                  
            /*if ((i==1 && j==6) || (i==6 && j==1))
            {
              printf("! %d %g %g %g %g %g %X %X %d %d\n", k, X[i][k], X[j][k], Delta[k], X[j][k] + Delta[k]*(ghost>>k & 1)*((ghostdir>>k & 1)?-1:1) + additionaldelta, additionaldelta, ghost, ghostdir, c, c2) ; 
            }*/
          }
                   
          if (sum<(r[i]+r[j])*(r[i]+r[j]))
          {
            //printf(".\n") ; fflush(stdout) ; 
            ncontact++ ; 
            if (i<j) {tmpcp.i = i ; tmpcp.j=j ; tmpcp.ghostdir = ghostdir ;}
            else    { tmpcp.i = j ; tmpcp.j=i ; tmpcp.ghostdir = (~ghostdir)&ghost ;} 
            tmpcp.contactlength=sqrt(sum) ; tmpcp.ghost=ghost ; 
            if (!cell_contact_insert(ID, CLp_it, tmpcp)) CLnew.v.push_back(tmpcp) ; 
          }
        }
    }
    
    //printf("]\n") ; 
  }
  //printf("E%d ", ID) ; fflush(stdout) ; 
  
  //printf("{%d %ld}", ncontact, CLnew.v.size()) ; fflush(stdout) ;  
  return 0 ; 
}
//-----------------------------------------------------------------------------
template <int d>
bool Cells<d>::cell_contact_insert (int ID, CLp_it_t<d> & CLp_it, const cp<d>& a)
 { 
     if (CLp_it.it_array_beg[a.i] == CLp_it.null_list.v.begin()) return false ; 
     //for (auto it = CLp_it.it_array_beg[a.i] ; it != CLp_it.it_array_end[a.i] ; it++)
     for (auto it = CLp_it.it_array_beg[a.i] ; it != CLp_it.it_ends[ID] && it->i == a.i ; it++)
     {
        if ((*it)==a)
        {
            it->contactlength=a.contactlength ;
            it->ghost=a.ghost ;
            it->ghostdir=a.ghostdir ;
            it->persisting = true ; 
            return true ; 
        }
     }
     return false ; 
 }

//========================================================================================
template <int d>
int Cells<d>::contacts_cell_cell (int ID, Cell & c1, Cell & c2, CLp_it_t<d> & CLp_it, 
                                  ContactList<d> & CLnew, 
                                  std::vector<std::vector<double>> const & X, std::vector<double> const &r, cp<d> & tmpcp)
{
 double sum ;
 for (size_t ii=0 ; ii<c1.incell.size() ; ii++)
   for (size_t jj=0 ; jj<c2.incell.size() ; jj++)
   {
      sum = 0 ; 
      int i = c1.incell[ii] ;
      int j = c2.incell[jj] ;
      for (int k=0 ; sum<(r[i]+r[j])*(r[i]+r[j]) && k<d ; k++) 
        sum+= (X[i][k]-X[j][k])*(X[i][k]-X[j][k]) ;
      if (sum<(r[i]+r[j])*(r[i]+r[j]))
      {
          if (i<j) {tmpcp.i = i ; tmpcp.j=j ;}
          else { tmpcp.i = j ; tmpcp.j=i ;} 
          tmpcp.contactlength=sqrt(sum) ; tmpcp.ghost=0 ; tmpcp.ghostdir=0 ;
          if (!cell_contact_insert(ID, CLp_it, tmpcp)) CLnew.v.push_back(tmpcp) ; 
      }
   }
  return 0;
}
//-------------------------------------------------------------------
template <int d>
int Cells<d>::contacts_cell_ghostcell (int ID, Cell & c1, Cell & c2, bitdim ghost, bitdim ghostdir, CLp_it_t<d> & CLp_it,
                                       ContactList<d> & CLnew, std::vector<std::vector<double>> const & X, std::vector<double> const &r, double LE_displacement, cp<d> & tmpcp)
{      
 double sum ; 
 for (size_t ii=0 ; ii<c1.incell.size() ; ii++)
   for (size_t jj=0 ; jj<c2.incell.size() ; jj++)
   {
     sum = 0 ; 
     int i = c1.incell[ii] ;
     int j = c2.incell[jj] ;
     for (int k=0 ; sum<(r[i]+r[j])*(r[i]+r[j]) && k<d ; k++) 
     {
       double additionaldelta=0 ; 
       if (k==1 && ghost&1) additionaldelta = LE_displacement * (ghostdir&1?-1:1) ;
       sum+= pow(( X[i][k]-X[j][k] - Delta[k]*(ghost>>k & 1)*((ghostdir>>k & 1)?-1:1) - additionaldelta),2) ;
     }
                   
     if (sum<(r[i]+r[j])*(r[i]+r[j]))
     {
       if (i<j) {tmpcp.i = i ; tmpcp.j=j ; tmpcp.ghostdir = ghostdir ;}
       else    { tmpcp.i = j ; tmpcp.j=i ; tmpcp.ghostdir = (~ghostdir)&ghost ;} 
       tmpcp.contactlength=sqrt(sum) ; tmpcp.ghost=ghost ; 
       if (!cell_contact_insert(ID, CLp_it, tmpcp)) CLnew.v.push_back(tmpcp) ; 
     }
  }
 return 0;
}
//=========================================================================================
template <int d>
int Cells<d>::contacts_external (int ID, Cells<d> &ocell, int delta_lvl, std::pair<int,int> bounds, CLp_it_t<d> & CLp_it,
                                 ContactList<d> & CLnew, std::vector<std::vector<double>> const & X, std::vector<double> const &r, double LE_displacement)
{
  auto [cell_first, cell_last] = bounds ; 
  cp<d> tmpcp(0,0,0,nullptr) ; tmpcp.persisting = true ; 

  for (int c=cell_first ; c<cell_last ; c++)
  {
    if (cells[c].incell.size()==0) continue ; 
    
    auto xloc = id2x(c) ; 
    for (auto &v:xloc) v /= pow(2, delta_lvl) ; 
    int oid = ocell.x2id(xloc) ; 
       
    // Outer cell contact
    contacts_cell_cell(ID, cells[c], ocell.cells[oid], CLp_it, CLnew, X, r, tmpcp) ; 
    
    for (size_t cc=0 ; cc<ocell.cells[oid].neighbours.size() ; cc++)
      contacts_cell_cell(ID, cells[c], ocell.cells[ocell.cells[oid].neighbours[cc]], CLp_it, CLnew, X, r, tmpcp) ; 
    for (size_t cc=0 ; cc<ocell.cells[oid].neighbours_full.size() ; cc++)
      contacts_cell_cell(ID, cells[c], ocell.cells[ocell.cells[oid].neighbours_full[cc]], CLp_it, CLnew, X, r, tmpcp) ; 
    
    //outer cell ghost contacts
    for (size_t cc=0 ; cc<ocell.cells[oid].neigh_ghosts.size() ; cc++)
    {
      int c2 = ocell.cells[oid].neigh_ghosts[cc] ; 
      bitdim ghost=ocell.cells[oid].neigh_ghosts_dim[cc], ghostdir=ocell.cells[oid].neigh_ghosts_dir[cc] ; 
      
      /*if (LE_displacement && ocell.cells[oid].neigh_ghosts_dim[cc]&1) // This is a ghost cell through a lees-edward => UNTESTED FOR OCTREE TODO
      {
        int newc2 = c2-floor(LE_displacement*(ocell.cells[oid].neigh_ghosts_dir[cc]&1?-1:1)/ocell.delta[1]) * ocell.cum_n_cell[1] ;
        c2 = clip_LEmoved_cell(c2, newc2, ghost, ghostdir) ;
      }*/
      
      contacts_cell_ghostcell(ID, cells[c], ocell.cells[c2], ghost, ghostdir, CLp_it, CLnew, X, r, LE_displacement, tmpcp) ; 
    }
    
    for (size_t cc=0 ; cc<ocell.cells[oid].neigh_ghosts_full.size() ; cc++)
    {
      int c2 = ocell.cells[oid].neigh_ghosts_full[cc] ; 
      bitdim ghost=ocell.cells[oid].neigh_ghosts_full_dim[cc], ghostdir=ocell.cells[oid].neigh_ghosts_full_dir[cc] ; 
      
      /*if (LE_displacement && ocell.cells[oid].neigh_ghosts_dim[cc]&1) // This is a ghost cell through a lees-edward => UNTESTED FOR OCTREE TODO
      {
        int newc2 = c2-floor(LE_displacement*(ocell.cells[oid].neigh_ghosts_dir[cc]&1?-1:1)/ocell.delta[1]) * ocell.cum_n_cell[1] ;
        c2 = clip_LEmoved_cell(c2, newc2, ghost, ghostdir) ;
      }*/
      
      contacts_cell_ghostcell(ID, cells[c], ocell.cells[c2], ghost, ghostdir, CLp_it, CLnew, X, r, LE_displacement, tmpcp) ; 
    }
  }
  return 0 ; 
}

//====================================================
/*template <int d>
int Cells<d>::contacts_hierarchy_below (Cells<d> &ocell, int delta_lvl, std::pair<int,int> bounds, ContactList<d> & CLall, ContactList<d> & CLnew, std::vector<std::vector<double>> const & X, std::vector<double> const &r, double LE_displacement)
{
  auto [cell_first, cell_last] = bounds ; 
  cp<d> tmpcp(0,0,0,nullptr) ; tmpcp.persisting = true ; 
  double sum=0 ;
  int ncontact=0 ; 
  for (int c=cell_first ; c<cell_last ; c++)
  {
    if (cells[c].incell.size()==0) continue ; 
    
    auto xloc = id2x(c) ; 
    for (auto &v:xloc) v /= pow(2, delta_lvl) ; 
    int oid = ocell.x2id(xloc) ; 
       
    // Outer cell contact
    if (c==170 && oid==1) printf("Checking L1-170 against L0-1") ; 
    for (size_t ii=0 ; ii<cells[c].incell.size() ; ii++)
      for (size_t jj=0 ; jj<ocell.cells[oid].incell.size() ; jj++)
      {
        sum = 0 ; 
        int i = cells[c].incell[ii] ;
        int j = ocell.cells[oid].incell[jj] ;
        for (int k=0 ; sum<(r[i]+r[j])*(r[i]+r[j]) && k<d ; k++) 
          sum+= (X[i][k]-X[j][k])*(X[i][k]-X[j][k]) ;
        if (sum<(r[i]+r[j])*(r[i]+r[j]))
        {
            ncontact++ ; 
            if (i<j) {tmpcp.i = i ; tmpcp.j=j ;}
            else { tmpcp.i = j ; tmpcp.j=i ;} 
            tmpcp.contactlength=sqrt(sum) ; tmpcp.ghost=0 ; tmpcp.ghostdir=0 ;
            if (!CLall.insert_cell(tmpcp)) CLnew.v.push_back(tmpcp) ; 
        }
      }
    
    //Outer cell neighbours contacts
    for (size_t cc=0 ; cc<ocell.cells[oid].neighbours.size() ; cc++)
    {
      int c2 = ocell.cells[oid].neighbours[cc] ; 
      if (c==194 && c2==1) printf("Checking L1-194 against L0-1") ; 
      for (size_t ii=0 ; ii<cells[c].incell.size() ; ii++)
        for (size_t jj=0 ; jj<ocell.cells[c2].incell.size() ; jj++)
        {
          sum = 0 ; 
          int i = cells[c].incell[ii] ;
          int j = ocell.cells[c2].incell[jj] ;
          for (int k=0 ; sum<(r[i]+r[j])*(r[i]+r[j]) && k<d ; k++) 
            sum+= (X[i][k]-X[j][k])*(X[i][k]-X[j][k]) ;
          if (sum<(r[i]+r[j])*(r[i]+r[j]))
          {
            ncontact++ ; 
            if (i<j) {tmpcp.i = i ; tmpcp.j=j ;}
            else { tmpcp.i = j ; tmpcp.j=i ;} 
            tmpcp.contactlength=sqrt(sum) ; tmpcp.ghost=0 ; tmpcp.ghostdir=0 ;
            if (!CLall.insert_cell(tmpcp)) CLnew.v.push_back(tmpcp) ; 
          }
        }
    }
    
    //outer cell ghost contacts
    for (size_t cc=0 ; cc<ocell.cells[oid].neigh_ghosts.size() ; cc++)
    {
      int c2 = ocell.cells[oid].neigh_ghosts[cc] ; 
      bitdim ghost=ocell.cells[oid].neigh_ghosts_dim[cc], ghostdir=ocell.cells[oid].neigh_ghosts_dir[cc] ; 
      
      if (LE_displacement && ocell.cells[oid].neigh_ghosts_dim[cc]&1) // This is a ghost cell through a lees-edward => UNTESTED FOR OCTREE
      {
        int newc2 = c2-floor(LE_displacement*(ocell.cells[oid].neigh_ghosts_dir[cc]&1?-1:1)/ocell.delta[1]) * ocell.cum_n_cell[1] ;
        c2 = clip_LEmoved_cell(c2, newc2, ghost, ghostdir) ;
      }
      
      for (size_t ii=0 ; ii<cells[c].incell.size() ; ii++)
        for (size_t jj=0 ; jj<ocell.cells[c2].incell.size() ; jj++)
        {
          double sum = 0 ; 
          int i = cells[c].incell[ii] ;
          int j = ocell.cells[c2].incell[jj] ;
          for (int k=0 ; sum<(r[i]+r[j])*(r[i]+r[j]) && k<d ; k++) 
          {
            double additionaldelta=0 ; 
            if (k==1 && ghost&1)
              additionaldelta = LE_displacement * (ghostdir&1?-1:1) ;
            sum+= pow(( X[i][k]-X[j][k] - Delta[k]*(ghost>>k & 1)*((ghostdir>>k & 1)?-1:1) - additionaldelta),2) ;
          }
                   
          if (sum<(r[i]+r[j])*(r[i]+r[j]))
          {
            //printf(".\n") ; fflush(stdout) ; 
            ncontact++ ; 
            if (i<j) {tmpcp.i = i ; tmpcp.j=j ; tmpcp.ghostdir = ghostdir ;}
            else    { tmpcp.i = j ; tmpcp.j=i ; tmpcp.ghostdir = (~ghostdir)&ghost ;} 
            tmpcp.contactlength=sqrt(sum) ; tmpcp.ghost=ghost ; 
            if (!CLall.insert_cell(tmpcp)) CLnew.v.push_back(tmpcp) ; 
          }
        }
    }
  }
  return 0 ; 
}*/

#endif
//===========================================================
/*int main(int argc, char * argv[])
{
  for (int i=0 ; i<4 ; i++)
    for (int j=0 ; j<4 ; j++)
      for (int k=0 ; k<4 ; k++)
      {
        long long id = hilbertDistance({i,j,k}, 4) ; 
        printf("%d %d %d | %lld \n", i,j,k, id) ; 
      }
  
  
  
  std::vector<std::vector<double>> bounds ; 
  for (int dd=0 ; dd<D ; dd++)
    bounds.push_back({0,1}) ; 
  double ds = 1/8. ; 
  
  boost::random::mt19937 rng(865); 
  boost::random::uniform_01<boost::mt19937> rand(rng) ; 
  auto start = std::chrono::high_resolution_clock::now();
  auto elapsed = std::chrono::high_resolution_clock::now()-start;   
  auto duration= std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count() ; 
  
  std::vector<std::vector<double>> X (N, std::vector<double>(D,0))  ; 
  for (auto & v : X)
      for (auto & w: v)
          w = rand() ;
  
  double Dbase = 0.05 ;  double Dsqr=Dbase*Dbase; 
  int count=0 ; 
  
  //-----------------------------------------------------------
  Cells<D> allcells; 
  start = std::chrono::high_resolution_clock::now();
  allcells.init_cells(bounds, ds) ; 
  elapsed = std::chrono::high_resolution_clock::now()-start; 
  duration= std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count() ; 
  printf("INIT duration: %g\n", duration/1000000.) ; 
  allcells.test_idempotent() ; 
  printf("INITIALISATION FINISHED\n") ; fflush(stdout) ;
  
  
  //------------------------------------------------------------
  start = std::chrono::high_resolution_clock::now();
  count = 0; 
  allcells.allocate_to_cells(X) ; 
  count = allcells.check_contact(X, Dsqr) ; 
  elapsed = std::chrono::high_resolution_clock::now()-start; 
  duration= std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count() ; 
  printf("CNT cells: %g %d\n", duration/1000000., count) ; 
  
  
  //------------------------------------------------------------
  start = std::chrono::high_resolution_clock::now();
  count =0 ;
  for (int i=0 ; i<N ; i++)
      for (int j=i+1 ; j<N ; j++)
      {
        double dst = allcells.dsqr(X[i], X[j]) ; 
        if (dst < Dsqr)
            count ++ ;
      }
  elapsed = std::chrono::high_resolution_clock::now()-start; 
  duration= std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count() ; 
  printf("CNT normal: %g %d\n", duration/1000000., count) ; 
  

}*/














