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


/** \brief Individual cell for contact detection
 */
class Cell {
public: 
  std::vector<int> incell ; ///< list of particles in the cell
  std::vector<int> neighbours ;  ///< list of cell neighbours to the current cell
} ;

/** \brief All the cells making the space, with related function for creating the cell array, neighbour arrays etc. 
 * Currently non implemented TODO: no pbc handling, no handling of moving boundaries. 
 */
template <int d>
class Cells {
public: 
  int init_cells (std::vector<std::vector<double>> &boundaries, double ds) ; 
  int init_cells (std::vector<std::vector<double>> &boundaries, std::vector<double> & r) {double ds = 2.1*(*std::max_element(r.begin(), r.end())) ; return (init_cells(boundaries, ds)) ; }
  std::vector<std::vector<double>> boundary_handler (std::vector<std::vector<double>> &boundaries) ;
  int allocate_to_cells (std::vector<std::vector<double>> & X) ; 
  int contacts (std::pair<int,int> bounds, ContactList<d> & CLall, ContactList<d> & CLnew, std::vector<std::vector<double>> const & X, std::vector<double> const &r) ; 
  //-------------------------------------------
  int x2id (const std::vector<int>&v) 
  {
    int res=0 ; 
    for (int dd=0; dd<d ; dd++)
    {
      if (v[dd] < 0 || v[dd]>=n_cell[dd]) {printf("WARN: particle outside the cell volume\n") ; fflush(stdout) ; return -1 ; }
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
  
  
  std::vector<int> n_cell , cum_n_cell ; 
  std::vector<double> origin ; 
  std::vector<Cell> cells ;
  double delta ; 
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
std::vector<std::vector<double>> Cells<d>::boundary_handler (std::vector<std::vector<double>> &boundaries)
{
  std::vector<std::vector<double>> res (d, std::vector<double>(2,0)) ;

  for (size_t i=0 ; i<boundaries.size() ; i++)
  {
    switch (static_cast<WallType>(boundaries[i][3]))
    {
      case WallType::PBC:
      case WallType::MOVINGWALL:
      case WallType::PBC_LE:
      case WallType::ELLIPSE:
                printf("ERR: this type of boundary is incompatible with cell contacts\n") ;
                break ;
      case WallType::WALL:
                res[i][0] = boundaries[i][0] ;
                res[i][1] = boundaries[i][1] ;
                break ;
      case WallType::SPHERE:
      case WallType::ROTATINGSPHERE:
                   for (int dd=0 ; dd<d ; dd++)
                   {
                      res[dd][0] = boundaries[i][4+dd]-boundaries[i][0] ;
                      res[dd][1] = boundaries[i][4+dd]+boundaries[i][0] ;
                   }
                   break ;
      case WallType::HEMISPHERE:
                   for (int dd=0 ; dd<d ; dd++)
                   {
                      res[dd][0] = boundaries[i][4+dd]-boundaries[i][0] ;
                      if (dd != boundaries[i][2])
                        res[dd][1] = boundaries[i][4+dd]+boundaries[i][0] ;
                   }
                   break ;
      case WallType::AXIALCYLINDER:
                   for (int dd=0 ; dd<d ; dd++)
                   {
                     if (dd==boundaries[i][1]) continue ;
                     res[dd][0] = boundaries[i][4+dd]-boundaries[i][0] ;
                     res[dd][1] = boundaries[i][4+dd]+boundaries[i][0] ;
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
int Cells<d>::init_cells(std::vector<std::vector<double>> &bounds, double ds)
{
  auto boundaries = boundary_handler(bounds) ;

  n_cell.resize(d,0);
  cum_n_cell.resize(d+1,0) ; 
  origin.resize(d,0) ;
  delta=ds ; 
  
  cum_n_cell[0] = 1 ;  
  for (int i=0 ; i<d ; i++)
  {
    n_cell[i] = ceil((boundaries[i][1]-boundaries[i][0])/ds) ; 
    if (i>0) 
      cum_n_cell[i] = cum_n_cell[i-1]*n_cell[i-1] ; 
    origin[i] = boundaries[i][0] ; 
  }
  cum_n_cell[d] = cum_n_cell[d-1]*n_cell[d-1] ; 
  
  cells.resize(cum_n_cell[d]) ; 
  for (auto &v: n_cell) printf("%d ", v) ;
  printf("\n") ; 
  for (auto &v: cum_n_cell) printf("%d ", v) ; 
  printf("\n") ; 
  
  int ntot = 1<<d ; 
  std::array<uint8_t, d> tmp ; 
  
  for (size_t i=0 ; i<cells.size() ; i++)
  {
    auto x_base = id2x(i) ; 
    auto x = x_base ; 
    for (int dd=0 ; dd<d ; dd++) tmp[dd] = x_base[dd] ; 
    
    /*auto res = hilbert::v2::PositionToIndex<uint8_t, d> (tmp) ; 
    for (auto & v: res) 
      printf("%d ", v) ; 
    printf("\n") ; */
    //cells[i].hilbert_idx = hilbert::v2::PositionToIndex<int, d> (tmp) ; 
    
    cells[i].neighbours.reserve(pow(3,d)) ;       
    
    for (int j=1 ; j<ntot ; j++)
    {
      int nct = __builtin_popcount(j) ; 
      int ndir = 1<<nct ; 
      for (int k=0 ; k<ndir ; k++)
      {
        bool noadd = false ; 
        x = x_base ; 
        
        int dirn=0 ; 
        for (int dd=0 ; dd<d ; dd++)
        {
          if (j>>dd & 1)
          {
            if (k>>dirn & 1) 
              x[dd] += 1 ;
            else
              x[dd] -= 1 ; 
            dirn++ ; 
            if (x[dd]<0 || x[dd]>=n_cell[dd]) 
            {
              noadd=true ; 
              break ; 
            }
          }
        }
        if (!noadd)
          cells[i].neighbours.push_back(x2id(x)) ;
      }
    }
    
    sort( cells[i].neighbours.begin(), cells[i].neighbours.end() );
    int bef = cells[i].neighbours.size() ; 
    cells[i].neighbours.erase( unique( cells[i].neighbours.begin(), cells[i].neighbours.end() ), cells[i].neighbours.end() );
    int aft = cells[i].neighbours.size() ; 
    if (bef-aft != 0) printf("ERR: no duplication of cells should happen with the algorithm.\n") ;  
    cells[i].neighbours.erase(std::remove_if(cells[i].neighbours.begin(), cells[i].neighbours.end(), 
                                            [=](size_t x) { return x<=i ; }), cells[i].neighbours.end());
  }
  
  /*for (int i=0 ; i<cum_n_cell[d] ; i++)
  {
    printf("%d ", cells[i].neighbours.size()) ;    
  }*/
  
  return 0 ; 
}
//=================================================================
template <int d>
int Cells<d>::allocate_to_cells (std::vector<std::vector<double>> & X)
{
  std::vector<int> v (d,0) ; 
  
  for (size_t i=0 ; i<cells.size() ; i++)
    cells[i].incell.clear() ; 
  
  for (size_t i=0 ; i<X.size() ; i++)
  {
    for (int dd=0 ; dd<d ; dd++)
      v[dd] = floor((X[i][dd]-origin[dd])/delta) ; 
    int id = x2id(v) ; 
    if (id != -1)
      cells[id].incell.push_back(i) ;
  } 
  
  /*int tot = 0 ; 
  for (size_t i=0 ; i<cells.size() ; i++)
    tot += cells[i].incell.size() ; */
  return 0 ; 
}
//=================================================================
template <int d>
int Cells<d>::contacts (std::pair<int,int> bounds, ContactList<d> & CLall, ContactList<d> & CLnew, std::vector<std::vector<double>> const & X, std::vector<double> const &r)
{
  auto [cell_first, cell_last] = bounds ; 
  cp<d> tmpcp(0,0,0,nullptr) ; tmpcp.persisting = true ; 
  double sum=0 ;
  int ncontact=0 ; 
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
            if (!CLall.insert_cell(tmpcp)) CLnew.v.push_back(tmpcp) ; 
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
            if (!CLall.insert_cell(tmpcp)) CLnew.v.push_back(tmpcp) ; 
          }
      }    
    }
  }
  
  //printf("{%d %ld}", ncontact, CLnew.v.size()) ; fflush(stdout) ;  
  return 0 ; 
}


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














