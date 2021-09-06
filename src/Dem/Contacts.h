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

    void particle_particle   ( cv1d & Xi, cv1d & Vi, cv1d &Omegai, double ri,
                               cv1d & Xj, cv1d & Vj, cv1d &Omegaj, double rj, cp & Contact) ; ///< Force & torque between 2 particles
    void particle_wall       ( cv1d & Vi, cv1d &Omegai, double ri,
                               cv1d & cn, cp & Contact) ; ///< Force & torque between a particle and a wall
    void particle_movingwall ( cv1d & Vi, cv1d & Omegai, double ri,
                               cv1d & cn, cv1d & Vj, cp & Contact) ; ///< Force & torque between a particle and a moving wall. Vj is the velocity of the wall at the contact point.
    void particle_ghost (cv1d & Xi, cv1d & Vi, cv1d &Omegai, double ri,
                              cv1d & Xj, cv1d & Vj, cv1d &Omegaj, double rj, cp & Contact, FILE * tmpfile)
    {
        loc=Xj ; 
        uint32_t gh=Contact.ghost, ghd=Contact.ghostdir ;
        for (int n=0 ; gh>0 ; gh>>=1, ghd>>=1, n++)
        {
          if (gh&1)
            loc[n] += P->Boundaries[n][2] * ((ghd&1)?-1:1) ;
        }
        
        //TESTING
        double DeltaLE = ((Contact.ghostdir&1)?-1:1) * P->Boundaries[0][4] * P->Boundaries[0][2] ; 
        if (tmpfile != nullptr)
            fprintf(tmpfile, "%g %g %g %g %g %d\n", loc[0], loc[1], Vj[0], Vj[1]+((Contact.ghost & 1)?1:0)*DeltaLE, rj, Contact.ghost) ; 
        #ifdef LEESEDWARD
        if ( (Contact.ghost & 1) && P->Boundaries[0][3]==static_cast<int>(WallType::PBC_LE))
        {
            double DeltaLE = ((Contact.ghostdir&1)?-1:1) * P->Boundaries[0][4] * P->Boundaries[0][2] ; 
            return particle_leesedwardghost (Xi, Vi, Omegai, ri, loc, Vj, Omegaj, rj, DeltaLE, Contact) ;
        }
        else
        #endif
          return particle_particle (Xi, Vi, Omegai, ri, loc, Vj, Omegaj, rj, Contact)  ; // Tricky, either a else case if LEESEDWARD is defined, or just always on. 
          
    } ///< Force and torque between a particle and a ghost (moves the ghost and calls particle_particle()
    #ifdef LEESEDWARD
    void particle_leesedwardghost (cv1d & Xi, cv1d & Vi, cv1d & Omegai, double ri,
                                                cv1d & Xj, cv1d & Vj, cv1d & Omegaj, double rj, double DeltaLE, cp & Contact) ; ///< Handle the case when a contact crosses a Lees-Edward boundary condition 
    #endif

    Action Act ; ///< Resulting Action

private:
  /** Temporary variables for internal use, to avoid reallocation at each call */
  
  double contactlength, ovlp, dotvrelcn, Coulomb  ;
  vector <double> cn, rri, rrj, vn, vt, Fn, Ft, tvec, Ftc, tspr ;
  #ifdef LEESEDWARD
  double Coulomb_i, Coulomb_j ; 
  vector <double> vnLE, vtLE, Fn_i, Fn_j, Ft_i, Ft_j, tspr_i, tspr_j ; 
  #endif

  vector <double> loc = vector<double>(d, 0) ;
  vector <double> vel = vector<double>(d, 0) ;
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
  tspr.resize(d,0) ;
  #ifdef LEESEDWARD
  vnLE.resize(d,0); vtLE.resize(d,0); 
  Fn_i.resize(d,0); Fn_j.resize(d,0); 
  Ft_i.resize(d,0); Ft_j.resize(d,0); 
  tspr_i.resize(d,0); tspr_j.resize(d,0) ; 
  #endif
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
  #ifndef LEESEDWARD  
  Act.set(Fn, Ft, Torquei, Torquej) ;
  #else 
  Act.set(Fn, Ft, -Fn, -Ft, Torquei, Torquej) ;
  #endif
  return ;
}

//---------------------- particle wall contact ----------------------------
template <int d>
void Contacts<d>::particle_wall ( cv1d & Vi, cv1d &Omegai, double ri,
                                  cv1d & cn, cp & Contact)
                                  //int j, int orient, cp & Contact)
{
  contactlength=Contact.contactlength ;
  ovlp=ri-contactlength ;
  if (ovlp<=0) {Act.setzero(d) ; return ;}

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
  Coulomb=P->Mu_wall*Tools<d>::norm(Fn) ;

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

  Contact.tspr=tspr ;
  Act.set(Fn, Ft, Torquei, Torquej) ;
  return ;
}

//---------------------- particle moving wall contact ----------------------------
template <int d>
void Contacts<d>::particle_movingwall (           cv1d & Vi, cv1d & Omegai, double ri,
                                       cv1d & cn, cv1d & Vj, cp & Contact)
{
  contactlength=Contact.contactlength ;

  ovlp=ri-contactlength ;
  if (ovlp<=0) {Act.setzero(d) ; return ;}

  //Relative velocity at contact
  Tools<d>::vMul(rri, cn, ovlp/2.-ri) ; // rri = -cn * (ri-ovlp/2.) ;
  vrel= Vi - Tools<d>::skewmatvecmult(Omegai, rri) - Vj ;
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
  Coulomb=P->Mu_wall*Tools<d>::norm(Fn) ;

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

  Contact.tspr=tspr ;
  Act.set(Fn, Ft, Torquei, Torquej) ;
  return ;
}

//=====================================================================
#ifdef LEESEDWARD
template <int d>
void Contacts<d>::particle_leesedwardghost (cv1d & Xi, cv1d & Vi, cv1d & Omegai, double ri,
                                            cv1d & Xj, cv1d & Vj, cv1d & Omegaj, double rj, double DeltaLE, cp & Contact)
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
  vrel= Vi - Tools<d>::skewmatvecmult(Omegai, rri) - (Vj - Tools<d>::skewmatvecmult(Omegaj, rrj)) ; //TODO optimize
  Tools<d>::vMul (vn, cn, Tools<d>::dot(vrel,cn)) ; //vn=cn * (Tools<d>::dot(vrel,cn)) ;
  Tools<d>::vMinus(vt, vrel, vn) ; //vt= vrel - vn ;
  
  vnLE = - cn*(DeltaLE*cn[1]) ; // vnLE = cn * dot(DeltaLE,cn)
  vtLE = - vnLE ; vtLE[1] -= DeltaLE ; 
  
  //Normal force
  Fn=cn*(ovlp*P->Kn) - vn*P->Gamman ; //TODO
  Fn_i =  Fn /*- vnLE*P->Gamman*/ ; 
  Fn_j = -Fn /*- vnLE*P->Gamman*/ ; 

  //Tangential force computation: retrieve contact or create new contact
  tspr_i=Contact.tspr_i ;
  if (tspr_i.size()==0) tspr_i.resize(d,0) ;
  tspr_j=Contact.tspr_j ;
  if (tspr_j.size()==0) tspr_j.resize(d,0) ;
  
  Tools<d>::vAddScaled (tspr_i, P->dt,  vt+vtLE) ; //tspradd_compile_definitions( += vt*dt ;
  Tools<d>::vAddScaled (tspr_j, P->dt, -vt/*+vtLE*/) ; //tspr += vt*dt ;
  Tools<d>::vSubScaled (tspr_i, Tools<d>::dot(tspr_i,cn), cn) ; // tspr -= cn * Tools<d>::dot(tspr,cn) ; //WARNING: might need an additional scaling so that |tsprnew|=|tspr|
  Tools<d>::vSubScaled (tspr_j, Tools<d>::dot(tspr_j,cn), cn) ; // tspr -= cn * Tools<d>::dot(tspr,cn) ; //WARNING: might need an additional scaling so that |tsprnew|=|tspr|
  
  Tools<d>::vMul(Ft_i, tspr_i, - P->Kt) ; //Ft=  tspr*(-Kt) ;
  Tools<d>::vMul(Ft_j, tspr_i, P->Kt) ; //Ft=  tspr*(-Kt) ;
  Coulomb_i=P->Mu*Tools<d>::norm(Fn_i) ;
  Coulomb_j=P->Mu*Tools<d>::norm(Fn_j) ;
  
  if (Tools<d>::norm(Ft_i) >= Coulomb_i)
  {
    if (Tools<d>::norm(tspr_i)>0)
    {
      Tools<d>::vMul(tvec, Ft_i, 1/Tools<d>::norm(Ft_i)) ; //tvec=Ft * (1/Tools<d>::norm(Ft)) ;
      Tools<d>::vMul(Ftc, tvec, Coulomb_i) ; //Ftc = tvec * Coulomb ;
      Ft_i=Ftc ;
      Tools<d>::vMul(tspr_i, Ftc, -1/ P->Kt) ; //tspr=Ftc*(-1/Kt) ;
    }
    else
      Tools<d>::setzero(Ft_i) ;
  }
  else
      Tools<d>::vSubScaled(Ft_i, P->Gammat, vt/*+vtLE*/) ; //Ft -= (vt*Gammat) ;
  
  // same for j, changing what's needed
  if (Tools<d>::norm(Ft_j) >= Coulomb_j)
  {
      if (Tools<d>::norm(tspr_i)>0)
      {
      Tools<d>::vMul(tvec, Ft_j, 1/Tools<d>::norm(Ft_j)) ; //tvec=Ft * (1/Tools<d>::norm(Ft)) ;
      Tools<d>::vMul(Ftc, tvec, Coulomb_j) ; //Ftc = tvec * Coulomb ;
      Ft_j=-Ftc ;
      //Tools<d>::vMul(tspr_i, Ftc, -1/ P->Kt) ; //tspr=Ftc*(-1/Kt) ;
      }
      else
        Tools<d>::setzero(Ft_j) ;
  }
  else
      Tools<d>::vSubScaled(Ft_j, P->Gammat, -vt/*+vtLE*/) ; //Ft -= (vt*Gammat) ;

  Tools<d>::wedgeproduct(Torquei, rri, Ft_i) ;
  Tools<d>::wedgeproduct(Torquej, rrj, Ft_j) ; //TODO check the minus sign

  //Update contact history
  //History[make_pair(i,j)]=make_pair (true, tspr) ;
  Contact.tspr_i=tspr_i ;
  Contact.tspr_j=tspr_j ; 
  Act.set(Fn_i, Ft_i, Fn_j, Ft_j, Torquei, Torquej) ;
  return ;
}
#endif



#endif
/** @} */
