#include "Typedefs.h"
#include "Parameters.h"
//#include "ContactList.h"
#include <boost/math/special_functions/binomial.hpp>

#ifndef MESH
#define MESH

template <int d>
class Mesh 
{
public: 
  Mesh (int dim, std::vector<std::vector<double>> vertices, bool compute_submeshes=true) ;
  void translate (cv1d t) {origin += t ; for (auto & v: submeshes) for (auto &w: v) w.translate(t) ; }
  void rotate (cv1d rot) 
  { 
    v1d res(d) ; 
    Tools<d>::matvecmult(res, rot, origin) ; 
    origin=res ; 
    for (auto & v: mixedbase)
    {
      Tools<d>::matvecmult(res, rot, v) ; 
      v=res ; 
    }
    for (auto & v: submeshes) 
      for (auto &w: v)
        w.rotate(rot) ; 
  }
  std::string export_json () 
  {
    std::string out, indent ;
    
    auto write_point= [](cv1d v){std::string res="[" ; for(int i=0 ; i<d ; i++) {res+= std::to_string(v[i]) ; if (i!=d-1) res+=", " ;} res +="]" ; return res ; } ;
    
     
    indent="  " ; 
    out +=  indent + "{\"dimensionality\":" + std::to_string(dimensionality) +",\n" + indent + "\"vertices\": [\n" ; 
    indent = indent + " " ; 
    out += indent + write_point(origin) ; 
    for (int i=d-dimensionality ; i<d ; i++)
      out += ", \n" + indent + write_point(origin+mixedbase[i]) ; 
    indent = "  " ; 
    out += "]\n" + indent + "}" ; 
    
    return out ;     
  }
  
  void disp () {
    for (int i=0 ; i<dimensionality ; i++)
    {
      printf("DIMENSIONALITY: %d\n", i) ; 
      for (auto & w: submeshes[i])
      {
        std::cout << "orig: ";
        for (auto &y:w.origin)
          std::cout << y << " ";
        std::cout << "\n" ; 
        for (auto &x:w.mixedbase)
        {
          for (auto &y: x)  
            std::cout << y << " " ; 
          std::cout << "\n";
        }
        std::cout << "\n";
      }
      std::cout << "--\n" ; 
    }}
  
  int dimensionality ; 
  std::vector<double> origin ; 
  std::vector<double> norms ; 
  std::vector<std::vector<double>> mixedbase ; ///< Mixed based: first n=dimensionality vectors are neither normalised nor unit vectors, while they d-n last vectors are. 
  std::vector<std::vector<Mesh>> submeshes ; 
} ; 


//====================================================================================
template <int d>
Mesh<d>::Mesh (int dim, std::vector<std::vector<double>> vertices, bool compute_submeshes)
{
  dimensionality=dim ; 
  if (vertices.size() != static_cast<long unsigned int>(dimensionality+1)) printf("ERR: incorrect number of vertices %ld for the mesh of dimensionality %d.\n", vertices.size(), dimensionality) ; 
  origin=vertices[0] ; 
  mixedbase.resize(d, std::vector<double>(d)) ;
  norms.resize(d, 0) ; 
  
  for (int i=1 ; i<dimensionality+1 ; i++)
  {
    mixedbase[i-1]=vertices[i]-vertices[0] ; 
    norms[i-1]=Tools<d>::norm(mixedbase[i-1]) ; 
  } 
  
  // Completing the mixed base with random vectors first
  //TODO reset random number
  boost::random::mt19937 rng(12345);
  boost::random::uniform_01<boost::mt19937> rand(rng) ;
  for (int i=dimensionality ; i<d-1 ; i++)
    for (int j=0 ; j<d ; j++)
      mixedbase[i][j]=rand() ; //TODO change random number 
  
  std::vector<double> tmp (d,0) ; 
  for (int i=dimensionality ; i<d ; i++)
  {
    for (int j=0 ; j<d ; j++)
    {
      Tools<d>::unitvec(mixedbase[d-1], j) ; 
      tmp[j] = Tools<d>::det(mixedbase) ;  
    }
    norms[i]=Tools<d>::norm(tmp) ; 
    mixedbase[i] = tmp/norms[i]; 
  }
  
  //printf("mixed base: %d | %g %g %g | %g %g %g | %g %g %g\n",dimensionality, mixedbase[0][0], mixedbase[0][1], mixedbase[0][2], mixedbase[1][0],mixedbase[1][1], mixedbase[1][2], mixedbase[2][0],mixedbase[2][1], mixedbase[2][2]) ;  
  //Reorganising the mixed base for performance reasons (probably not a huge improvement, would be interesting to actually test
  std::rotate(mixedbase.begin(), mixedbase.begin()+dimensionality, mixedbase.end()) ; 
  std::rotate(norms.begin(), norms.begin()+dimensionality, norms.end()) ; 
  
  // Handling lower dimensionality sides
  if (compute_submeshes)
  {    
    submeshes.resize(dimensionality) ; 
    for (int i=1 ; i<(1<<(dimensionality+1))-1 ; i++)
    {
      int subdimensionality = __builtin_popcount(i)-1 ; 
      std::vector<std::vector<double>> pts (subdimensionality+1, std::vector<double>(d)) ; 
      for (int j=0, n=0 ; j<dimensionality+1 ; j++)
      {
        if ((i>>j) &1) 
          pts[n++]=vertices[j] ;
      }
      submeshes[subdimensionality].push_back(Mesh(subdimensionality, pts, false)) ; 
    }
  
    for (int i=0 ; i<dimensionality-1 ; i++)
      if (submeshes[i].size() != boost::math::binomial_coefficient<double>(dimensionality+1, i+1))
        printf("ERR: the wrong number of submeshes %ld of dimensionality %d was generated ...\n", submeshes[i].size(), i) ; 
      
  }
}



#endif

















