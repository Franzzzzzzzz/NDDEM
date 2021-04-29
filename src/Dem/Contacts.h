/** \addtogroup DEM Discrete Element Simulations
 * This module handles the Discrete Element Simulations.
 *  @{ */

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

/** \brief Calculate contact forces
 */
template <int d>
class Contacts
{
public:
    Contacts (Parameters<d> &PP) ;

    //int N ; ///< Number of particles // These and following should really be const but that's a pain for assignements
    Parameters<d> *P ; ///< Pointer to the Parameters structure.
    vector < double > Torquei ; ///< Torque on particle i
    vector < double > Torquej ; ///< Torque on particle j
    vector < double > vrel ; ///< Relative velocity

    void particle_particle (cv1d & Xi, cv1d & Vi, cv1d &Omegai, double ri,
                              cv1d & Xj, cv1d & Vj, cv1d &Omegaj, double rj, cp & Contact) ; ///< Force & torque between 2 particles
    void particle_wall     ( cv1d & Vi, cv1d &Omegai, double ri,
                                 int j, int orient, cp & Contact) ; ///< Force & torque between a particle and a wall
    void particle_ghost (cv1d & Xi, cv1d & Vi, cv1d &Omegai, double ri,
                              cv1d & Xj, cv1d & Vj, cv1d &Omegaj, double rj, cp & Contact)
    {
        vector <double> loc (d, 0) ;
        loc=Xj ;
        uint32_t gh=Contact.ghost, ghd=Contact.ghostdir ;
        for (int n=0 ; gh>0 ; gh>>=1, ghd>>=1, n++)
        {
          if (gh&1)
            loc[n] += P->Boundaries[n][2] * ((ghd&1)?-1:1) ;
        }
        return (particle_particle (Xi, Vi, Omegai, ri, loc, Vj, Omegaj, rj, Contact) ) ;
    } ///< Force and torque between an particle and a ghost (moves the ghost and calls particle_particle()

    Action Act ; ///< Resulting Action

private:
  /** Temporary variables for internal use, to avoid reallocation at each call */
  double contactlength, ovlp, dotvrelcn, Coulomb, tsprn, tsprcn ;
  vector <double> cn, rri, rrj, vn, vt, Fn, Ft, tvec, Ftc, tspr, tsprc ;

} ;


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
Contacts<d>::Contacts (Parameters<d> &PP) : P(&PP)
{

  Torquei.resize(d*(d-1)/2, 0) ;
  Torquej.resize(d*(d-1)/2, 0) ;
  vrel.resize(d,0) ;
  cn.resize(d,0) ;
  rri.resize(d,0) ; rrj.resize(d,0) ; vn.resize(d,0) ; vt.resize(d,0) ;
  Fn.resize(d,0) ; Ft.resize(d,0) ; tvec.resize(d,0) ; Ftc.resize(d,0) ;
  tspr.resize(d,0) ; tsprc.resize(d,0) ;

}

//--------------------------------------------------------------------------------------
//---------------------- particle particle contact ----------------------------
template <int d>
void Contacts<d>::particle_particle (cv1d & Xi, cv1d & Vi, cv1d & Omegai, double ri,
                                     cv1d & Xj, cv1d & Vj, cv1d & Omegaj, double rj, cp & Contact)
{
  contactlength=Contact.contactlength ;

  ovlp=ri+rj-contactlength ;
  if (ovlp<=0) {Act.setzero(d) ; return ;}
  //printf("%g %g %g %g\n", ri, rj, Xi[2], Xj[2]) ; fflush(stdout) ;
  Tools<d>::vMinus(cn, Xi, Xj) ; //cn=(Xi-Xj) ;
  cn /= contactlength ;

  //Relative velocity at contact
  Tools<d>::vMul(rri, cn, ovlp/2.-ri) ; // rri = -cn * (ri-ovlp/2.) ;
  Tools<d>::vMul(rrj, cn, rj-ovlp/2.) ; // rrj =  cn * (rj-ovlp/2.) ;
  vrel= Vi - Tools<d>::skewmatvecmult(Omegai, rri) - (Vj - Tools<d>::skewmatvecmult(Omegaj, rrj)) ; //TODO
  Tools<d>::vMul (vn, cn, Tools<d>::dot(vrel,cn)) ; //vn=cn * (Tools<d>::dot(vrel,cn)) ;
  Tools<d>::vMinus(vt, vrel, vn) ; //vt= vrel - vn ;

  //Normal force
  Fn=cn*(ovlp*P->Kn) - vn*P->Gamman ; //TODO

  //Tangential force computation: retrieve contact or create new contact
  tspr=Contact.tspr ;
  if (tspr.size()==0) tspr.resize(d,0) ;

  Tools<d>::vAddScaled (tspr, P->dt, vt) ; //tspr += vt*dt ;
  Tools<d>::vSubScaled(tspr, Tools<d>::dot(tspr,cn), cn) ; // tspr -= cn * Tools<d>::dot(tspr,cn) ; //WARNING: might need an additional scaling so that |tsprnew|=|tspr|
  Tools<d>::vMul(Ft, tspr, - P->Kt) ; //Ft=  tspr*(-Kt) ;
  Coulomb=P->Mu*Tools<d>::norm(Fn) ;

  if (Tools<d>::norm(Ft) >= Coulomb)
  {
    if (Tools<d>::norm(tspr)>0)
    {
      Tools<d>::vMul(tvec, Ft, 1/Tools<d>::norm(Ft)) ; //tvec=Ft * (1/Tools<d>::norm(Ft)) ;
      Tools<d>::vMul(Ftc, tvec, Coulomb) ; //Ftc = tvec * Coulomb ;
      Ft=Ftc ;
      Tools<d>::vMul(tspr, Ftc, -1/ P->Kt) ; //tspr=Ftc*(-1/Kt) ;
    }
    else
      Tools<d>::setzero(Ft) ;
  }
  else
      Tools<d>::vSubScaled(Ft, P->Gammat, vt) ; //Ft -= (vt*Gammat) ;

  Tools<d>::wedgeproduct(Torquei, rri, Ft) ;
  Tools<d>::wedgeproduct(Torquej, rrj, -Ft) ; //TODO check the minus sign

  //Update contact history
  //History[make_pair(i,j)]=make_pair (true, tspr) ;
  Contact.tspr=tspr ;
  Act.set(Fn, Ft, Torquei, Torquej) ;
  return ;
}

//---------------------- particle wall contact ----------------------------
template <int d>
void Contacts<d>::particle_wall ( cv1d & Vi, cv1d &Omegai, double ri,
                               int j, int orient, cp & Contact)
{
  contactlength=Contact.contactlength ;
  ovlp=ri-contactlength ;
  if (ovlp<=0) {Act.setzero(d) ; return ;}
  Tools<d>::unitvec(cn, j) ;
  cn=cn*(-orient) ; // l give the orientation (+1 or -1)

  //Relative velocity at contact
  rri = -cn * (ri-ovlp/2.) ;
  vrel= Vi - Tools<d>::skewmatvecmult(Omegai, rri) ;
  vn = cn * (Tools<d>::dot(vrel, cn)) ;
  vt= vrel - vn ; //vt=self.vrel-vn*cn ; // BUG: from python, must be wrong
  Fn=cn*(ovlp*P->Kn) - vn*P->Gamman ;

  //Tangential force computation: retrieve contact or create new contact
  //history=History[make_pair(i,-(2*j+k+1))] ;
  tspr=Contact.tspr ; //history.second ;
  if (tspr.size()==0) tspr.resize(d,0) ;

  tspr += vt*P->dt ;
  tspr -= cn * Tools<d>::dot(tspr,cn) ; //WARNING: might need an additional scaling so that |tsprnew|=|tspr| Actually does not seem to change anything ...
  Ft=  tspr*(- P->Kt) ;
  Coulomb=P->Mu*Tools<d>::norm(Fn) ;

  if (Tools<d>::norm(Ft) > Coulomb)
  {
    if (Tools<d>::norm(tspr)>0)
    {
      tvec=Ft * (1/Tools<d>::norm(Ft)) ;
      Ftc = tvec * Coulomb ;
      Ft=Ftc ;
      tspr=Ftc*(-1/ P->Kt) ;
    }
    else
      Tools<d>::setzero(Ft) ;
  }
  else
     Ft -= (vt*P->Gammat) ;

  Torquei=Tools<d>::wedgeproduct(rri, Ft) ;
  Torquei-= Omegai*0.00005 ; // damping 
  //History[make_pair(i,-(2*j+k+1))]=make_pair (true, tspr) ;
  Contact.tspr=tspr ;
  Act.set(Fn, Ft, Torquei, Torquej) ;
  return ;
}

#endif
/** @} */
