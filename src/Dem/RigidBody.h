
#include <algorithm>
#include <optional>

template <int d>
class RigidBody 
{
public:
  RigidBody() {com=std::vector<double>(d) ; F=std::vector<double>(d,0) ;}
  RigidBody(std::vector<int> iid, cv2d & X, cv1d & m) 
  {
    com=std::vector<double>(d,0) ;
    F=std::vector<double>(d,0) ;
    mass=0 ; 
    ids=iid ;
    std::sort(ids.begin(), ids.end()) ; 
     
    
    for (auto &i:ids)
    {
      com += (X[i]*m[i]) ;
      mass += m[i] ; 
    }
    for (auto & v:com) v/=mass ; 
  }
  
  //variables
  std::vector<int> ids ; 
  v1d F ; 
  v1d com ; 
  double mass ; 
  
  
} ;

template <int d>
class RigidBodies_
{
public :

  std::vector<RigidBody<d>> RB ; 
  
  int add_body (std::vector<int> v, cv2d &X, cv1d &m) {RB.push_back(RigidBody<d>(v,X,m)); return 0;}
  
  int allocate (std::vector<std::optional<int>> & RigidBodyId)
  {
    for (size_t i=0 ; i<RB.size() ; i++)
    {
      for (auto & j: RB[i].ids)
      {
        if (RigidBodyId[j].has_value())
          printf("WARN: grain %d may be allocated to several rigid bodies ...\n", j) ;
        RigidBodyId[j]=i ; 
      }
    }
    return 0;
  }
  
  int process_forces (std::vector < std::vector <double> > & F, std::vector < std::vector <double> > & Torque, cv1d m)
  {
    for (auto & v: RB)
    {
      Tools<d>::setzero(v.F) ; 
      
      for (auto &id : v.ids) v.F += F[id] ;
      for (auto &id : v.ids)
      { 
        for (int dd=0 ; dd<d ; dd++)
          F[id][dd]=v.F[dd]/v.mass*m[id] ; 
        Tools<d>::setzero(Torque[id]) ;  
      }
    }
    return 0; 
  }
  
  
  
};




