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
extern long int cid ;
class Contacts
{
public:
    Contacts (Parameters &P) ;
    void clean_history () ;

    const int d, N ;
    const double dt, Kn, Kt, Mu, Gamman, Gammat ;
    const Parameters *ptrP ;
    vector < double > Torquei,Torquej, vrel ;

    void particle_particle (cv1d & Xi, cv1d & Vi, cv1d Omegai, double ri,
                              cv1d & Xj, cv1d & Vj, cv1d Omegaj, double rj, cp & Contact) ;
    void particle_wall     ( cv1d & Xi, cv1d & Vi, cv1d Omegai, double ri,
                                 int j, int orient, cp & Contact) ;
    void particle_ghost (cv1d & Xi, cv1d & Vi, cv1d Omegai, double ri,
                              cv1d & Xj, cv1d & Vj, cv1d Omegaj, double rj, cp & Contact)
    {
        static vector <double> gst (d, 0) ;
        gst=Xj ;
        gst[abs(Contact.isghost)-1] += ptrP->Boundaries[abs(Contact.isghost)-1][2]*(Tools::sgn(Contact.isghost)) ;
        return (particle_particle (Xi, Vi, Omegai, ri, gst, Vj, Omegaj, rj, Contact) ) ;
    }

    Action Act ;

private:
  // Temporary variables for internal use, to avoid reallocation at each call
  double contactlength, ovlp, dotvrelcn, Coulomb, tsprn, tsprcn ;
  vector <double> cn, rri, rrj, vn, vt, Fn, Ft, tvec, Ftc, tspr, tsprc ;

} ;

#endif
