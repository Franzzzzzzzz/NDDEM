#include <vector>
#include <list>
#include "Typedefs.h"
#include "Parameters.h"
#include "Tools.h"


#ifndef CONTACTLIST
#define CONTACTLIST

// ------------------------------------ Action class -------------------------------------------
class Action {
public :
    vector <double> Fn, Ft, Torquei, Torquej ;
    void set (v1d a, v1d b, v1d c, v1d d) {Fn=a ; Ft=b ; Torquei=c ; Torquej=d ; }
    void setzero (int d) {Fn=(v1d(d,0)) ; Ft=(v1d(d,0)) ; Torquei=(v1d(d*(d-1)/2,0)) ; Torquej=(v1d(d*(d-1)/2)) ; }
} ;


// ------------------------------------ Contact properties class -------------------------------------------
class cp // Contact properties class
{
public:
 cp (int ii, int jj, int d, double ctlength, Action * default_action) : i(ii), j(jj), contactlength(ctlength), tspr (vector <double> (d, 0)), infos(default_action), owninfos(false){} //creator
 ~cp () { if (owninfos) delete(infos) ; }
 cp & operator= (const cp & c)
 {
     i=c.i ;
     if (c.i<0) return *this ; //is a deleted element, just keep moving
     j=c.j ; // copy everything else if it is not a deleted element
     ghost=c.ghost ;
     ghostdir=c.ghostdir ;
     tspr=c.tspr ;
     contactlength=c.contactlength ;
     infos=c.infos ;
     return *this ; 
 }
 Action & getinfo () {return *infos ; }
 //void setinfo (Action & a) {if (!infos) infos = new Action ; *infos=a ; }
 void setinfo (Action * a) {infos=a ; }

 int i, j ;
 double contactlength ;
 //int8_t isghost ;        // LIMIT d<128
 u_int32_t ghost, ghostdir ;
 vector <double> tspr;
 Action * infos ;
 bool owninfos ;
} ;

// ------------------------------------ Contact List class -------------------------------------------
template <int d>
class ContactList
{
public:
 ContactList () : deltamap(vector<double>(d,0)),masking(vector<u_int32_t>(d,0)),pbcdim(vector<int>(d,0)) {}
 void reset() {it = v.begin() ;}
 int insert(const cp& a) ;
 void finalise () { while (it!=v.end()) it=v.erase(it) ; }
 list <cp> v ;
 Action * default_action () {return (&def) ; }
 int cid=0 ;
 vector <double> deltamap ; 
 vector <u_int32_t> masking ; 
 vector <int> pbcdim ; 

 //void check_ghost    (u_int32_t gst, double partialsum, const Parameters & P, cv1d &X1, cv1d &X2, double R, cp & tmpcp) ;
 void check_ghost_dst(u_int32_t gst, int n, double partialsum, u_int32_t mask, const Parameters<d> & P, cv1d &X1, cv1d &X2, cp & contact) ; 
 void check_ghost (bitdim gst, const Parameters<d> & P, cv1d &X1, cv1d &X2, cp & tmpcp,
                   int startd=0, double partialsum=0, bitdim mask=0) ; 
 void coordinance (v1d &Z) ; 

private:
 list<cp>::iterator it ;
 Action def ;
};

inline bool operator< (const cp &a, const cp &b) {if (a.i==b.i) return (a.j<b.j) ; return a.i<b.i ; }
inline bool operator== (const cp &a, const cp &b) {return (a.i==b.i && a.j==b.j) ; }


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
int ContactList<d>::insert(const cp &a)
{
    while (it!=v.end() && (*it) < a)
    { it= v.erase(it) ; }
    if (it==v.end()) {v.push_back(a) ; it=v.end() ; }
    else
    {
        if ((*it)==a)
        {
            it->contactlength=a.contactlength ;
            it->ghost=a.ghost ;
            it->ghostdir=a.ghostdir ;
            it++ ;
        }
        else {it=v.insert(it,a) ; it++ ; }
    }
    return (cid++) ;
}

//-----------------------------------Fastest version so far ...
template <int d>
void ContactList<d>::check_ghost (bitdim gst, const Parameters<d> & P, cv1d &X1, cv1d &X2, cp & tmpcp, 
                                 int startd, double partialsum, bitdim mask)
{
    double sum=partialsum ;  
    for (int dd=startd ; sum<P.skinsqr && dd<d ; dd++, gst>>=1)
    {
        sum += (X1[dd]-X2[dd]) * (X1[dd]-X2[dd]) ; 
        if (gst & 1)
        {
            double Delta= (tmpcp.ghostdir&(1<<dd)?-1:1) * P.Boundaries[dd][2] ;
            double sumspawn = partialsum + (X1[dd]-X2[dd]-Delta) * (X1[dd]-X2[dd]-Delta) ;
            if (sumspawn<P.skinsqr)
                check_ghost (gst>>1, P, X1, X2, tmpcp, dd+1, sumspawn, mask | (1<<dd)) ; 
        }
        partialsum = sum ; 
    } 
    if (sum<P.skinsqr)
    {
        tmpcp.contactlength=sqrt(sum) ;
        tmpcp.ghost=mask ; 
        insert(tmpcp) ;
    }
}

//----------------------------------------
template <int d>
void ContactList<d>::check_ghost_dst(u_int32_t gst, int n, double partialsum, u_int32_t mask, const Parameters<d> & P, cv1d &X1, cv1d &X2, cp & contact)
{
  if (gst==0) return ;
  else
  {
    for ( ;(gst&1)==0; gst>>=1,n++) ;
    check_ghost_dst(gst-1, n, partialsum, mask, P, X1, X2, contact) ;
    double Delta= (contact.ghostdir&(1<<n)?-1:1) * P.Boundaries[n][2] ;
    partialsum = partialsum + Delta*(2*(X2[n]-X1[n]) + Delta) ;
    if (partialsum<contact.contactlength) // Found a lower distance with this ghost
    {
      contact.contactlength=partialsum ;
      contact.ghost = mask|(1<<n) ;
    }
    check_ghost_dst(gst-1, n, partialsum, mask|(1<<n), P, X1, X2, contact) ;
  }
}

//-----------------------------------
template <int d>
void ContactList<d>::coordinance (v1d &Z)
{
  for (auto & w : v)
  {
      Z[w.i]++ ; Z[w.j] ++ ; 
  }
}

#endif
