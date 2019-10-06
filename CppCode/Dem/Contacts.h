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
    Contacts (Parameters &P) ;

    int d, N ; // These and following should really be const but that's a pain for assignements
    double dt, Kn, Kt, Mu, Gamman, Gammat ;
    Parameters *ptrP ;
    vector < double > Torquei,Torquej, vrel ;

    void particle_particle (cv1d & Xi, cv1d & Vi, cv1d &Omegai, double ri,
                              cv1d & Xj, cv1d & Vj, cv1d &Omegaj, double rj, cp & Contact) ;
    void particle_wall     ( cv1d & Vi, cv1d &Omegai, double ri,
                                 int j, int orient, cp & Contact) ;
    void particle_ghost (cv1d & Xi, cv1d & Vi, cv1d &Omegai, double ri,
                              cv1d & Xj, cv1d & Vj, cv1d &Omegaj, double rj, cp & Contact)
    {
        static vector <double> loc (d, 0) ;
        loc=Xj ;
        u_int32_t gh=Contact.ghost, ghd=Contact.ghostdir ;
        for (int n=0 ; gh>0 ; gh>>=1, ghd>>=1, n++)
        {
          if (gh&1)
            loc[n] += ptrP->Boundaries[n][2] * ((ghd&1)?-1:1) ;
        }
        return (particle_particle (Xi, Vi, Omegai, ri, loc, Vj, Omegaj, rj, Contact) ) ;
    }

    Action Act ;

private:
  // Temporary variables for internal use, to avoid reallocation at each call
  double contactlength, ovlp, dotvrelcn, Coulomb, tsprn, tsprcn ;
  vector <double> cn, rri, rrj, vn, vt, Fn, Ft, tvec, Ftc, tspr, tsprc ;

} ;

#endif
