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

    void particle_particle   ( cv1d & Xi, cv1d & Vi, cv1d &Omegai, double ri, double mi,
                               cv1d & Xj, cv1d & Vj, cv1d &Omegaj, double rj, double mj, cp<d> & Contact, bool isdumptime) ; ///< Force & torque between 2 particles
    void particle_wall       ( cv1d & Vi, cv1d &Omegai, double ri, double mi,
                               cv1d & cn, cp<d> & Contact) ; ///< Force & torque between a particle and a wall
    void particle_movingwall ( cv1d & Vi, cv1d & Omegai, double ri, double mi,
                               cv1d & cn, cv1d & Vj, cp<d> & Contact) ; ///< Force & torque between a particle and a moving wall. Vj is the velocity of the wall at the contact point.
    void particle_mesh       ( cv1d & Xi, cv1d & Vi, cv1d &Omegai, double ri, double mi, cpm<d> & Contact) ; ///< Force & torque between particle and mesh
    void (Contacts::*particle_ghost) (cv1d & Xi, cv1d & Vi, cv1d &Omegai, double ri, double mi,
                                      cv1d & Xj, cv1d & Vj, cv1d &Omegaj, double rj, double mj, cp<d> & Contact, bool isdumptime) ; ///< Function pointer to the function to calculate the ghost-to-particle contact forces

    void particle_ghost_regular (cv1d & Xi, cv1d & Vi, cv1d &Omegai, double ri, double mi,
                                 cv1d & Xj, cv1d & Vj, cv1d &Omegaj, double rj, double mj, cp<d> & Contact, bool isdumptime)
    {
        vector <double> loc (d, 0) ;
        loc=Xj ;
        uint32_t gh=Contact.ghost, ghd=Contact.ghostdir ;
        for (int n=0 ; gh>0 ; gh>>=1, ghd>>=1, n++)
        {
          if (gh&1)
            loc[n] += P->Boundaries[n][2] * ((ghd&1)?-1:1) ;
        }
        return (particle_particle (Xi, Vi, Omegai, ri, mi, loc, Vj, Omegaj, rj, mj, Contact, isdumptime) ) ;
    } ///< Calculate the particle to regular (non Lees-Edward) ghost contact



    void particle_ghost_LE (cv1d & Xi, cv1d & Vi, cv1d &Omegai, double ri, double mi,
                            cv1d & Xj, cv1d & Vj, cv1d &Omegaj, double rj, double mj, cp<d> & Contact, bool isdumptime) /*, FILE * debug = nullptr*/
    {
        vector <double> loc (d, 0) ;
        //if (debug == nullptr) n= 1 ;

        if ( P->Boundaries[0][3] != static_cast<int>(WallType::PBC_LE) || (Contact.ghost & 1)==0)
        {
            loc=Xj ;
            compute_normalpbcloc(loc, 0, Contact.ghost, Contact.ghostdir) ;
            particle_particle (Xi, Vi, Omegai, ri, mi, loc, Vj, Omegaj, rj, mj, Contact, isdumptime) ;
            //if (debug != nullptr)
            // fprintf(debug, "%g %g %g %g %g 0 %d\n", loc[0], loc[1], Vj[0], Vj[1], rj, Contact.j) ;
        }
        else
        {
         uint32_t gh=Contact.ghost, ghd=Contact.ghostdir ; // Handle pbc in first dim
         vector <double> vel (d, 0) ; vel=Vj ;
         loc=Xj ;
         loc[0] += P->Boundaries[0][2] * ((ghd&1)?-1:1) ;
         loc[1] += (ghd&1?-1:1)*P->Boundaries[0][5] ;
         double additionaldelta = 0 ;
         if (loc[1] > P->Boundaries[1][1]) {additionaldelta = -P->Boundaries[1][2] ;}
         if (loc[1] < P->Boundaries[1][0]) {additionaldelta =  P->Boundaries[1][2] ;}
         loc[1] += additionaldelta ;
         vel[1] += (ghd&1?-1:1) * P->Boundaries[0][4] * P->Boundaries[0][2] ;

         gh>>=1 ; ghd>>=1 ;
         compute_normalpbcloc (loc, 1, gh, ghd) ;
         particle_particle (Xi, Vi, Omegai, ri, mi, loc, vel, Omegaj, rj, mj, Contact, isdumptime) ;
         //if (debug != nullptr)
         //    fprintf(debug, "%g %g %g %g %g %d %d\n", loc[0], loc[1], vel[0], vel[1], rj, n, Contact.j) ;

        // And then backward ... ... ... but actually not needed yay!
         /*gh=Contact.ghost;
         ghd=~Contact.ghostdir ;
         loc = Xi ;
         vel=Vi ;
         loc[0] += P->Boundaries[0][2] * ((ghd&1)?-1:1) ;
         loc[1] += (ghd&1?-1:1)*P->Boundaries[0][5] ;
         loc[1] -= additionaldelta ;
         vel[1] += (ghd&1?-1:1) * P->Boundaries[0][4] * P->Boundaries[0][2] ;

         gh>>=1 ; ghd>>=1 ;
         compute_normalpbcloc (loc, 1, gh, ghd) ;
         particle_particle (Xj, Vj, Omegaj, rj, loc, vel, Omegai, ri, Contact, ContactCalculation::REVERSE) ;
         if (debug != nullptr)
             fprintf(debug, "%g %g %g %g %g %d %d\n", loc[0], loc[1], vel[0], vel[1], ri, -n, Contact.i) ;
         n++ ;
         printf("%g %g %g %g | %g %g %g %g\n", Act.Fni[0], Act.Fnj[0], Act.Fti[0], Act.Ftj[0],  Act.Fni[1], Act.Fnj[1], Act.Fti[1], Act.Ftj[1]) ; */
        }
    } ///< Calculate the particle to (potentially) Lees-Edward ghost contact

    void compute_normalpbcloc (v1d & loc, int startn, uint32_t gh, uint32_t ghd)
    {
        for (int n=startn ; gh>0 ; gh>>=1, ghd>>=1, n++)
        {
          if (gh&1)
            loc[n] += P->Boundaries[n][2] * ((ghd&1)?-1:1) ;
        }
    }///< Move ghosts through the regular periodic boundary conditions (non Lees-Edward).
    
    Action<d> Act ; ///< Resulting Action

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

  particle_ghost = &Contacts::particle_ghost_LE ;

}

//--------------------------------------------------------------------------------------
//---------------------- particle particle contact ----------------------------
template <int d>
void Contacts<d>::particle_particle (cv1d & Xi, cv1d & Vi, cv1d & Omegai, double ri, double mi,
                                     cv1d & Xj, cv1d & Vj, cv1d & Omegaj, double rj, double mj, cp<d> & Contact, bool isdumptime)
{
  double kn, kt, gamman, gammat ;
  contactlength=Contact.contactlength ;

  ovlp=ri+rj-contactlength ;
  if (ovlp<=0) {Act.setzero() ; return ;}
  //printf("%g %g %g %g\n", ri, rj, Xi[2], Xj[2]) ; fflush(stdout) ;
  Tools<d>::vMinus(cn, Xi, Xj) ; //cn=(Xi-Xj) ;
  cn /= contactlength ;

  //Relative velocity at contact
  Tools<d>::vMul(rri, cn, ovlp/2.-ri) ; // rri = -cn * (ri-ovlp/2.) ;
  Tools<d>::vMul(rrj, cn, rj-ovlp/2.) ; // rrj =  cn * (rj-ovlp/2.) ;
  vrel= Vi - Tools<d>::skewmatvecmult(Omegai, rri) - (Vj - Tools<d>::skewmatvecmult(Omegaj, rrj)) ; //TODO
  Tools<d>::vMul (vn, cn, Tools<d>::dot(vrel,cn)) ; //vn=cn * (Tools<d>::dot(vrel,cn)) ;
  Tools<d>::vMinus(vt, vrel, vn) ; //vt= vrel - vn ;

  Act.setvel(vn, vt) ;

  if (P->ContactModel == HERTZ)
  {
      double Rstar = (ri*rj)/(ri+rj) ;
      double Mstar = (mi*mj)/(mi+mj) ;
      double sqrtovlpR = sqrt(ovlp*Rstar) ;
      kn = P->Kn * sqrtovlpR;
      kt = P->Kt * sqrtovlpR;
      gamman = P->Gamman * sqrt(Mstar * kn);
      gammat = P->Gammat * sqrt(Mstar * kt);
  }
  else
  {
    kn=P->Kn ;
    kt=P->Kt ;
    gamman = P->Gamman ;
    gammat = P->Gammat ;
  }

  //Normal force
  Fn=cn*(ovlp*kn) - vn*gamman ;
  if (isdumptime)
  {
    Act.Fn_el = cn*(ovlp*kn) ; 
    Act.Fn_visc = - vn*gamman ; 
  }
  

  //Tangential force computation: retrieve contact or create new contact
  tspr=Contact.tspr ;
  if (tspr.size()==0) tspr.resize(d,0) ;

  Tools<d>::vAddScaled (tspr, P->dt, vt) ; //tspr += vt*dt ;
  Tools<d>::vSubScaled(tspr, Tools<d>::dot(tspr,cn), cn) ; // tspr -= cn * Tools<d>::dot(tspr,cn) ; //WARNING: might need an additional scaling so that |tsprnew|=|tspr|
  Tools<d>::vMul(Ft, tspr, - kt) ; //Ft=  tspr*(-Kt) ;
  Coulomb=P->Mu*Tools<d>::norm(Fn) ;

  if (Tools<d>::norm(Ft) >= Coulomb)
  {
    if (Tools<d>::norm(tspr)>0)
    {
      Tools<d>::vMul(tvec, Ft, 1/Tools<d>::norm(Ft)) ; //tvec=Ft * (1/Tools<d>::norm(Ft)) ;
      Tools<d>::vMul(Ftc, tvec, Coulomb) ; //Ftc = tvec * Coulomb ;
      Ft=Ftc ;
      Tools<d>::vMul(tspr, Ftc, -1/ kt) ; //tspr=Ftc*(-1/Kt) ;
    }
    else
      Tools<d>::setzero(Ft) ;
    
    if (isdumptime) {Act.Ft_fric = Ft ; Act.Ft_isfric=true ; } 
  }
  else
  {
    if (isdumptime) {Act.Ft_el = Ft ; Act.Ft_visc=-vt*gammat ; Act.Ft_isfric=false ; }
    Tools<d>::vSubScaled(Ft, gammat, vt) ; //Ft -= (vt*Gammat) ;
  }
  Tools<d>::wedgeproduct(Torquei, rri, Ft) ;
  Tools<d>::wedgeproduct(Torquej, rrj, -Ft) ; //TODO check the minus sign

  Contact.tspr=tspr ;
  //Act.set_rev(-Fn, -Ft, Torquej) ;
  //Act.set_fwd(Fn, Ft, Torquei) ;
  Act.set(Fn, Ft, Torquei, Torquej) ;
  return ;
}

//---------------------- particle wall contact ----------------------------
template <int d>
void Contacts<d>::particle_wall ( cv1d & Vi, cv1d &Omegai, double ri, double mi,
                                  cv1d & cn, cp<d> & Contact)
                                  //int j, int orient, cp & Contact)
{
  contactlength=Contact.contactlength ;
  ovlp=ri-contactlength ;
  if (ovlp<=0) {Act.setzero() ; return ;}

  //Relative velocity at contact
  rri = -cn * (ri-ovlp/2.) ;
  vrel= Vi - Tools<d>::skewmatvecmult(Omegai, rri) ;
  vn = cn * (Tools<d>::dot(vrel, cn)) ;
  vt= vrel - vn ;
  Act.setvel(vn, vt) ;

  double kn, kt, gamman, gammat ;
  if (P->ContactModel == HERTZ)
  {
      double sqrtovlpR = sqrt(ovlp*ri) ;
      kn = P->Kn * sqrtovlpR;
      kt = P->Kt * sqrtovlpR;
      gamman = P->Gamman * mi * sqrtovlpR;
      gammat = P->Gammat * mi * sqrtovlpR;
  }
  else
  {
    kn=P->Kn ;
    kt=P->Kt ;
    gamman = P->Gamman ;
    gammat = P->Gammat ;
  }

  Fn=cn*(ovlp*kn) - vn*gamman;

  //Tangential force computation: retrieve contact or create new contact
  //history=History[make_pair(i,-(2*j+k+1))] ;
  tspr=Contact.tspr ; //history.second ;
  if (tspr.size()==0) tspr.resize(d,0) ;

  tspr += vt*P->dt ;
  tspr -= cn * Tools<d>::dot(tspr,cn) ; //WARNING: might need an additional scaling so that |tsprnew|=|tspr| Actually does not seem to change anything ...
  Ft=  tspr*(- kt) ;
  Coulomb=P->Mu_wall*Tools<d>::norm(Fn) ;

  if (Tools<d>::norm(Ft) > Coulomb)
  {
    if (Tools<d>::norm(tspr)>0)
    {
      tvec=Ft * (1/Tools<d>::norm(Ft)) ;
      Ftc = tvec * Coulomb ;
      Ft=Ftc ;
      tspr=Ftc*(-1/ kt) ;
    }
    else
      Tools<d>::setzero(Ft) ;
  }
  else
     Ft -= (vt*gammat) ;

  Torquei=Tools<d>::wedgeproduct(rri, Ft) ;

  Contact.tspr=tspr ;
  Act.set(Fn, Ft, Torquei, Torquej) ;
  return ;
}

//---------------------- particle moving wall contact ----------------------------
template <int d>
void Contacts<d>::particle_movingwall ( cv1d & Vi, cv1d & Omegai, double ri, double mi,
                                        cv1d & cn, cv1d & Vj, cp<d> & Contact)
{
  contactlength=Contact.contactlength ;

  ovlp=ri-contactlength ;
  if (ovlp<=0) {Act.setzero() ; return ;}

  //Relative velocity at contact
  Tools<d>::vMul(rri, cn, ovlp/2.-ri) ; // rri = -cn * (ri-ovlp/2.) ;
  vrel= Vi - Tools<d>::skewmatvecmult(Omegai, rri) - Vj ;
  Tools<d>::vMul (vn, cn, Tools<d>::dot(vrel,cn)) ; //vn=cn * (Tools<d>::dot(vrel,cn)) ;
  Tools<d>::vMinus(vt, vrel, vn) ; //vt= vrel - vn ;
  Act.setvel(vn, vt) ;

  double kn, kt, gamman, gammat ;
  if (P->ContactModel == HERTZ)
  {
      double sqrtovlpR = sqrt(ovlp*ri) ;
      kn = P->Kn * sqrtovlpR;
      kt = P->Kt * sqrtovlpR;
      gamman = P->Gamman * mi * sqrtovlpR;
      gammat = P->Gammat * mi * sqrtovlpR;
  }
  else
  {
    kn=P->Kn ;
    kt=P->Kt ;
    gamman = P->Gamman ;
    gammat = P->Gammat ;
  }

  //Normal force
  Fn=cn*(ovlp*kn) - vn*gamman ; //TODO

  //Tangential force computation: retrieve contact or create new contact
  tspr=Contact.tspr ;
  if (tspr.size()==0) tspr.resize(d,0) ;

  Tools<d>::vAddScaled (tspr, P->dt, vt) ; //tspr += vt*dt ;
  Tools<d>::vSubScaled(tspr, Tools<d>::dot(tspr,cn), cn) ; // tspr -= cn * Tools<d>::dot(tspr,cn) ; //WARNING: might need an additional scaling so that |tsprnew|=|tspr|
  Tools<d>::vMul(Ft, tspr, - kt) ; //Ft=  tspr*(-Kt) ;
  Coulomb=P->Mu_wall*Tools<d>::norm(Fn) ;

  if (Tools<d>::norm(Ft) >= Coulomb)
  {
    if (Tools<d>::norm(tspr)>0)
    {
      Tools<d>::vMul(tvec, Ft, 1/Tools<d>::norm(Ft)) ; //tvec=Ft * (1/Tools<d>::norm(Ft)) ;
      Tools<d>::vMul(Ftc, tvec, Coulomb) ; //Ftc = tvec * Coulomb ;
      Ft=Ftc ;
      Tools<d>::vMul(tspr, Ftc, -1/ kt) ; //tspr=Ftc*(-1/Kt) ;
    }
    else
      Tools<d>::setzero(Ft) ;
  }
  else
      Tools<d>::vSubScaled(Ft, gammat, vt) ; //Ft -= (vt*Gammat) ;

  Tools<d>::wedgeproduct(Torquei, rri, Ft) ;

  Contact.tspr=tspr ;
  Act.set(Fn, Ft, Torquei, Torquej) ;
  return ;
}

//------------------------- Particle mesh contact --------------------------------------
template <int d>
void Contacts<d>::particle_mesh ( cv1d & Xi, cv1d & Vi, cv1d &Omegai, double ri, double mi, cpm<d> & Contact)
{
  contactlength=Contact.contactlength ;

  ovlp=ri-contactlength ;
  if (ovlp<=0) {Act.setzero() ; return ;}
  Tools<d>::vMinus(cn, Xi, Contact.contactpoint) ; //cn=(Xi-Xj) ;
  cn /= contactlength ;

  //Relative velocity at contact
  Tools<d>::vMul(rri, cn, ovlp/2.-ri) ; // rri = -cn * (ri-ovlp/2.) ;
  vrel= Vi - Tools<d>::skewmatvecmult(Omegai, rri) ;
  Tools<d>::vMul (vn, cn, Tools<d>::dot(vrel,cn)) ; //vn=cn * (Tools<d>::dot(vrel,cn)) ;
  Tools<d>::vMinus(vt, vrel, vn) ; //vt= vrel - vn ;
  Act.setvel(vn, vt) ;
  
  double kn, kt, gamman, gammat ;
  if (P->ContactModel == HERTZ)
  {
      double sqrtovlpR = sqrt(ovlp*ri) ;
      kn = P->Kn * sqrtovlpR;
      kt = P->Kt * sqrtovlpR;
      gamman = P->Gamman * mi * sqrtovlpR;
      gammat = P->Gammat * mi * sqrtovlpR;
  }
  else
  {
    kn=P->Kn ;
    kt=P->Kt ;
    gamman = P->Gamman ;
    gammat = P->Gammat ;
  }

  //Normal force
  Fn=cn*(ovlp*kn) - vn*gamman ; //TODO

  //Tangential force computation: retrieve contact or create new contact
  //history=History[make_pair(i,-(2*j+k+1))] ;
  tspr=Contact.tspr ; //history.second ;
  if (tspr.size()==0) tspr.resize(d,0) ;

  tspr += vt*P->dt ;
  tspr -= cn * Tools<d>::dot(tspr,cn) ; //WARNING: might need an additional scaling so that |tsprnew|=|tspr| Actually does not seem to change anything ...
  Ft=  tspr*(- kt) ;
  Coulomb=P->Mu_wall*Tools<d>::norm(Fn) ;

  if (Tools<d>::norm(Ft) > Coulomb)
  {
    if (Tools<d>::norm(tspr)>0)
    {
      tvec=Ft * (1/Tools<d>::norm(Ft)) ;
      Ftc = tvec * Coulomb ;
      Ft=Ftc ;
      tspr=Ftc*(-1/ kt) ;
    }
    else
      Tools<d>::setzero(Ft) ;
  }
  else
     Ft -= (vt*gammat) ;

  Torquei=Tools<d>::wedgeproduct(rri, Ft) ;

  Contact.tspr=tspr ;
  Act.set(Fn, Ft, Torquei, Torquej) ;
  return ;
}



#endif
/** @} */
