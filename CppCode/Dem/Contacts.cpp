

#include "Contacts.h"


Contacts::Contacts (Parameters &P) : d(P.d), N(P.N), dt(P.dt), Kn(P.Kn), Kt(P.Kt), Mu(P.Mu), Gamman(P.Gamman), Gammat(P.Gammat), ptrP(&P)
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
void Contacts::particle_particle (cv1d & Xi, cv1d & Vi, cv1d & Omegai, double ri,
                                     cv1d & Xj, cv1d & Vj, cv1d & Omegaj, double rj, cp & Contact)
{
  contactlength=Contact.contactlength ;

  ovlp=ri+rj-contactlength ;
  if (ovlp<=0) {Act.setzero(d) ; return ;}
  //printf("%g %g %g %g\n", ri, rj, Xi[2], Xj[2]) ; fflush(stdout) ;
  Tools::vMinus(cn, Xi, Xj) ; //cn=(Xi-Xj) ;
  cn /= contactlength ;

  //Relative velocity at contact
  Tools::vMul(rri, cn, ovlp/2.-ri) ; // rri = -cn * (ri-ovlp/2.) ;
  Tools::vMul(rrj, cn, rj-ovlp/2.) ; // rrj =  cn * (rj-ovlp/2.) ;
  vrel= Vi - Tools::skewmatvecmult(Omegai, rri) - (Vj - Tools::skewmatvecmult(Omegaj, rrj)) ; //TODO
  Tools::vMul (vn, cn, Tools::dot(vrel,cn)) ; //vn=cn * (Tools::dot(vrel,cn)) ;
  Tools::vMinus(vt, vrel, vn) ; //vt= vrel - vn ;

  //Normal force
  Fn=cn*(ovlp*Kn) - vn*Gamman ; //TODO

  //Tangential force computation: retrieve contact or create new contact
  tspr=Contact.tspr ;
  if (tspr.size()==0) tspr.resize(d,0) ;

  Tools::vAddScaled (tspr, dt, vt) ; //tspr += vt*dt ;
  Tools::vSubScaled(tspr, Tools::dot(tspr,cn), cn) ; // tspr -= cn * Tools::dot(tspr,cn) ; //WARNING: might need an additional scaling so that |tsprnew|=|tspr|
  Tools::vMul(Ft, tspr, -Kt) ; //Ft=  tspr*(-Kt) ;
  Coulomb=Mu*Tools::norm(Fn) ;

  if (Tools::norm(Ft) >= Coulomb)
  {
    if (Tools::norm(tspr)>0)
    {
      Tools::vMul(tvec, Ft, 1/Tools::norm(Ft)) ; //tvec=Ft * (1/Tools::norm(Ft)) ;
      Tools::vMul(Ftc, tvec, Coulomb) ; //Ftc = tvec * Coulomb ;
      Ft=Ftc ;
      Tools::vMul(tspr, Ftc, -1/Kt) ; //tspr=Ftc*(-1/Kt) ;
    }
    else
      Tools::setzero(Ft) ;
  }
  else
      Tools::vSubScaled(Ft, Gammat, vt) ; //Ft -= (vt*Gammat) ;

  Tools::wedgeproduct(Torquei, rri, Ft) ;
  Tools::wedgeproduct(Torquej, rrj, -Ft) ; //TODO check the minus sign

  //Update contact history
  //History[make_pair(i,j)]=make_pair (true, tspr) ;
  Contact.tspr=tspr ;
  Act.set(Fn, Ft, Torquei, Torquej) ;
  return ;
}

//---------------------- particle wall contact ----------------------------
void Contacts::particle_wall ( cv1d & Xi, cv1d & Vi, cv1d &Omegai, double ri,
                                 int j, int orient, cp & Contact)
{
  contactlength=Contact.contactlength ;
  ovlp=ri-contactlength ;
  if (ovlp<=0) {Act.setzero(d) ; return ;}
  Tools::unitvec(cn, d, j) ;
  cn=cn*(-orient) ; // l give the orientation (+1 or -1)

  //Relative velocity at contact
  rri = -cn * (ri-ovlp/2.) ;
  vrel= Vi - Tools::skewmatvecmult(Omegai, rri) ;
  vn = cn * (Tools::dot(vrel, cn)) ;
  vt= vrel - vn ; //vt=self.vrel-vn*cn ; // BUG: from python, must be wrong
  Fn=cn*(ovlp*Kn) - vn*Gamman ;

  //Tangential force computation: retrieve contact or create new contact
  //history=History[make_pair(i,-(2*j+k+1))] ;
  tspr=Contact.tspr ; //history.second ;
  if (tspr.size()==0) tspr.resize(d,0) ;

  tspr += vt*dt ;
  tspr -= cn * Tools::dot(tspr,cn) ; //WARNING: might need an additional scaling so that |tsprnew|=|tspr| Actually does not seem to change anything ...
  Ft=  tspr*(-Kt) ;
  Coulomb=Mu*Tools::norm(Fn) ;

  if (Tools::norm(Ft) > Coulomb)
  {
    if (Tools::norm(tspr)>0)
    {
      tvec=Ft * (1/Tools::norm(Ft)) ;
      Ftc = tvec * Coulomb ;
      Ft=Ftc ;
      tspr=Ftc*(-1/Kt) ;
    }
    else
      Tools::setzero(Ft) ;
  }
  else
     Ft -= (vt*Gammat) ;

  Torquei=Tools::wedgeproduct(rri, Ft) ;

  //History[make_pair(i,-(2*j+k+1))]=make_pair (true, tspr) ;
  Contact.tspr=tspr ;
  Act.set(Fn, Ft, Torquei, Torquej) ;
  return ;
}