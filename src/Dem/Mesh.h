
#include <boost/math/special_functions/binomial.hpp>

template <int d>
class Mesh 
{
public: 
  Mesh (int dim, std::vector<std::vector<double>> vertices)
  {
    dimensionality=dim ; 
    if (vertices.size() != static_cast<long unsigned int>(dimensionality+1)) printf("ERR: incorrect number of vertices %ld for the mesh of dimensionality %d.\n", vertices.size(), dimensionality) ; 
    origin=vertices[0] ; 
    mixedbase.resize(d, std::vector<double>(d)) ;
    
    for (int i=1 ; i<dimensionality+1 ; i++)
      mixedbase[i-1]=vertices[i]-vertices[0] ; 
    
    // Completing the mixed base with random vectors first
    //TODO reset random number
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
      mixedbase[i] = tmp/Tools<d>::norm(tmp) ; 
    }
    
    // Handling lower dimensionality sides
    for (int i=1 ; i<(1<<(dimensionality+1)) ; i++)
    {
     if (__builtin_popcount(i)==dimensionality)
     {
       std::vector<std::vector<double>> pts (dimensionality, std::vector<double>(d)) ; 
       for (int j=0, n=0 ; j<dimensionality+1 ; j++)
         if ((i>>j) &1) 
           pts[n++]=vertices[j] ;
       submeshes.push_back(Mesh(dimensionality-1, pts)) ; 
     }
    }
    
    if (submeshes.size() != boost::math::binomial_coefficient<double>(dimensionality+1, dimensionality))
      printf("ERR: the wrong number of submeshes was generated ...\n") ; 
  }
  
  //------------------------------------------------------
  std::tuple<double, int, std::vector<double>> contactdetection (cv1d Xo, double r) // TODO WORK IN PROGRESS
  {
    std::vector<double> dotproducts (d) ;
    auto X = Xo-origin ;
    double dstsqr=0 ; 
    
    /*for (int i=dimensionality ; i<d ; i++)
    {
      dotproducts[i]=Tools<d>::dot(mixedbase[i], X) ; 
      dstsqr+= dotproducts[i] ; 
    }
    if (dstsqr<r*r) //Potential contact
    {
      double Xl = Tools<d>::norm(X) ; 
      std::vector<double> coefficient(d) ;
      double coefficient_sum = 0 ;
      for (int i=0 ; i<dimensionality ; i++)
      {
        dotproducts[i] = Tools<d>::dot(mixedbase[i], X) ; 
        coefficient[i] = dotproducts[i]/norms[i]/Xl ; 
        if (coefficient[i]<0 || coefficient[i]>1) // We are outside
          return false ;  
        coefficient_sum += coefficient[i] ; 
      }
      if (coefficient_sum>=0 && coefficient_sum<=1) // Seems like we got a contact!
      {
        std::vector<double> contact_point (d, 0) ;
        for (int i=0 ; i<dimensionality ; i++)
          contact_point += coefficient[i]*mixedbase[i] ; 
        std::vector<double> cn = X - contact_point ;
        cn /= Tools<d>::norm(cn) ; 
      }
    }*/
  }
  
  int dimensionality ; 
  std::vector<double> origin ; 
  std::vector<std::vector<double>> mixedbase ; ///< Mixed based: first n=dimensionality vectors are neither normalised nor unit vectors, while they d-n last vectors are. 
  std::vector<Mesh> submeshes ; 
} ; 



















