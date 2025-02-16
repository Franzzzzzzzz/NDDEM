#include <iostream>
#include <fstream>

#ifndef BOUNDARIES_H
#define BOUNDARIES_H

template <int d>
class Boundary {
public:  
  WallType Type = WallType::UNDEFINED ; 
  double xmin, xmax, delta, velmin, velmax, displacement; 
  // would be nice as aliases, but too hard
  double vel ; 
  double semiaxisx, semiaxisy, centerx, centery ; 
  double radius ; 
  
  // not aliases ... 
  int axis ; 
  std::vector<double> center = std::vector<double>(d,0) ; 
  std::vector<double> omega  = std::vector<double>(d*(d-1)/2,0) ;   
  
  template <class Archive>
    void serialize(Archive &ar) {
        ar(Type, xmin, xmax, delta, velmin, velmax, displacement, vel, semiaxisx, semiaxisy, centerx, centery, axis, center, omega) ; 
    }  
  
  //============================================================================
  std::vector<double> as_vector() 
  {
    std::vector<double> res ; 
    switch (Type)
    {
      case WallType::PBC : 
        return {xmin, xmax, delta, static_cast<double>(Type)} ; 
      case WallType::PBC_LE :        
        return {xmin, xmax, delta, static_cast<double>(Type), vel} ; 
      case WallType::WALL:
        return {xmin, xmax, delta, static_cast<double>(Type)} ; 
      case WallType::MOVINGWALL :         
        return {xmin, xmax, delta, static_cast<double>(Type), velmin, velmax} ; 
      case WallType::SPHERE:
        res = {radius, radius*radius, 0, static_cast<double>(Type) } ; 
        res.insert(res.end(), center.begin(), center.end()) ; 
        return res ;         
      case WallType::ROTATINGSPHERE:
        res = {radius, radius*radius, 0, static_cast<double>(Type) } ; 
        res.insert(res.end(), center.begin(), center.end()) ; 
        res.insert(res.end(), omega.begin(), omega.end()) ; 
        return res ;         
      case WallType::HEMISPHERE:
        return {radius, radius*radius, 0, static_cast<double>(Type) } ; 
      case WallType::AXIALCYLINDER:
        return {radius, radius*radius, 0, static_cast<double>(Type) } ; 
      case WallType::ELLIPSE:
        return {semiaxisx, semiaxisy, 0, static_cast<double>(Type), centerx, centery} ; 
      case WallType::UNDEFINED:
        return {0, 0, 0, static_cast<double>(Type)} ; 
      default:
        printf("ERR Incorrect wall reading, unknown wall.\n") ; 
    }
    
  }
  
  int read_line (std::istream &in)
  {
    switch (Type)
    {
      case WallType::PBC : 
      case WallType::PBC_LE :        
      case WallType::WALL:
      case WallType::MOVINGWALL : 
                in >> xmin >> xmax ; 
                delta = xmax - xmin ; 
                break ; 
      case WallType::SPHERE:
      case WallType::ROTATINGSPHERE:
      case WallType::HEMISPHERE:
      case WallType::AXIALCYLINDER:
                in >> radius ; 
                break ; 
      case WallType::ELLIPSE:
                assert((d==2)) ; 
                in >> semiaxisx >> semiaxisy; 
                break ;
      case WallType::UNDEFINED:
      default:
        printf("ERR Incorrect wall reading, unknown wall.\n") ; 
    }
    switch (Type)
    {
      case WallType::AXIALCYLINDER : 
      case WallType::HEMISPHERE : 
                in >> axis ; 
                break ; 
      case WallType::PBC_LE :
                in >> vel ; 
                displacement = 0;
                break ; 
      case WallType::MOVINGWALL:
                in >> velmin >> velmax ; 
                break ; 
      case WallType::ELLIPSE:
                in >> centerx >> centery ; 
                break ; 
      case WallType::PBC:
      case WallType::SPHERE:
      case WallType::ROTATINGSPHERE:  
      case WallType::WALL:
      case WallType::UNDEFINED:
                break ; 
        
    }
    switch (Type)
    {
      case WallType::SPHERE:
      case WallType::ROTATINGSPHERE:
      case WallType::HEMISPHERE:
                for (int dd =0 ; dd<d ; dd++)
                  in >> center[dd] ; 
                break ; 
      case WallType::PBC:
      case WallType::PBC_LE:        
      case WallType::WALL: 
      case WallType::MOVINGWALL : 
      case WallType::AXIALCYLINDER : 
      case WallType::ELLIPSE:
      case WallType::UNDEFINED:
                break ; 
    
    }    
    if (Type == WallType::ROTATINGSPHERE)
      for (int i=0; i<d*(d-1)/2 ; i++) 
        in >> omega[i] ;
        
    return 0 ; 
  }
  
  bool is_sphere () {return (Type==WallType::SPHERE || Type == WallType::ROTATINGSPHERE);}
  bool is_periodic () {return (Type==WallType::PBC || Type == WallType::PBC_LE);}

} ; 





#endif
