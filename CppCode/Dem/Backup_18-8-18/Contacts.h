#ifndef CONTACTS
#define CONTACTS

#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <vector>
#include <cmath>
#include <map>
#include "Typedefs.h"
#include "Parameters.h"
#include "Tools.h"
#include "ContactList.h"

class Contacts
{
public:  
    Contacts (Parameters P) ;
    void clean_history () ; 
    
    int d, N ; 
    double dt, Kn, Kt, Mu, Gamman, Gammat ; 
    vector < double > Torquei,Torquej, vrel ; 
    map < pair <int,int>, pair <bool, v1d> > History ; 

    Action particle_particle (cv1d & Xi, cv1d & Vi, cv1d Omegai, double ri,
                              cv1d & Xj, cv1d & Vj, cv1d Omegaj, double rj, cp & Contact) ;  
    Action particle_wall     ( cv1d & Xi, cv1d & Vi, cv1d Omegai, double ri,
                                 int j, int orient, cp & Contact) ;
    Action particle_ghost (cv1d & Xi, cv1d & Vi, cv1d Omegai, double ri, // Just an alias
                              cv1d & Xj, cv1d & Vj, cv1d Omegaj, double rj, cp & Contact) {return (particle_particle (Xi, Vi, Omegai, ri, Xj, Vj, Omegaj, rj, Contact) ) ; }

private: 
  // Temporary variables for internal use, to avoid reallocation at each call 
  pair <bool, vector <double> > history ; 
  double contactlength, ovlp, dotvrelcn, Coulomb, tsprn, tsprcn ; 
  vector <double> cn, rri, rrj, vn, vt, Fn, Ft, tvec, Ftc, tspr, tsprc ; 
  
} ; 

#endif
