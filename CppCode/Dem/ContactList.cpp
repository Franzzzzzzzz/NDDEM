#include "ContactList.h"




//---------------------------------------
int ContactList::insert(const cp &a)
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


//--------------------------------------
void ContactList::check_ghost (u_int32_t gst, int n, double partialsum, u_int32_t mask, const Parameters & P, cv1d &X1, cv1d &X2, double R, cp & tmpcp)
{
  if (gst==0) return ;
  else
  {
    for ( ;(gst&1)==0; gst>>=1,n++) ;
    check_ghost(gst-1, n, partialsum, mask, P, X1, X2, R, tmpcp) ; //Recursing without changing partial sum -> ie. skipping over this PBC and moving to the next
    double Delta= (tmpcp.ghostdir&(1<<n)?-1:1) * P.Boundaries[n][2] ;
    partialsum = partialsum + Delta*(2*(X2[n]-X1[n]) + Delta) ;
    //printf("[%X %g %g %g]\n", tmpcp.ghostdir, P.Boundaries[n][2], sqrt(partialsum), sqrt(R)) ;
    if (partialsum<R)
    {
     //tmpcp.i=i ; tmpcp.j=j ; //must be set up before calling
     tmpcp.contactlength=sqrt(partialsum) ;
     tmpcp.ghost=mask|(1<<n) ;
     //tmpcp.ghostdir must be set up before calling
     insert(tmpcp) ;
     //printf("CONTACT\n") ;
    }
    check_ghost(gst-1, n, partialsum, mask|(1<<n), P, X1, X2, R, tmpcp ) ; // Recursing after changing partial sum -> looking for ghosts of ghosts...
  }
}

//----------------------------------------
void ContactList::check_ghost_dst(u_int32_t gst, int n, double partialsum, u_int32_t mask, const Parameters & P, cv1d &X1, cv1d &X2, cp & contact)
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




//=================================================================
// Sparse vector implementation, probably shouldn't use
//-----------------------------------------------
/*void sparsevector::insert_in_place (vector <cp> & v1) //WORKS
{
    if (vin.size()==0) return ;
    int last=v1.size()-1 ;
    v1.resize(last+1+vin.size(), vin[0]) ;
    for (int i=location.size()-1 ; i>=0 ; i--)
    {
        for (int j=last ; j>=0 && j>=location[i] ; j--)
            v1[j+i+1]=v1[j] ;
        v1[location[i]+i+1]=vin[i] ;
        last=location[i] ;
    }
}
//-------------------------------------
void sparsevector::postpone_insertion(cp & a, int idx)
{
    vin.push_back(a) ;
    location.push_back(idx) ;
}
//-------------------------------------
void sparsevector::compact (vector <cp> & v)
{
 int i, j ;
 printf("><") ;
 for (i=0, j=0 ; i<v.size() ; i++)
 {
     if (isdeleted(v[i]))
         continue ;
     v[j]=v[i] ; j++ ;
 }
 if (j>=i) printf("sparsevector compaction failed") ;
 v.resize(j,v[0]) ;
}
//-------------------------------------------------------
void sparsevector::finalise (vector<cp> &v)
{
 for (int i=idx ; i<v.size() ; i++) {v[i].i=-1 ;}
 insert_in_place(v) ;
 if (v.size()>max_size) compact(v) ;
 vin.clear() ; location.clear() ;
}
//-------------------------------------------------------------
int sparsevector::find_next_insertion (vector <cp> &v, cp &a)
{
    while (idx<v.size() && v[idx]<a)
    {v[idx].i=-1 ; idx++ ;}
    if (idx==v.size()) {v.push_back(a) ; idx++;}
    else
    {
     if (v[idx]==a)
         idx++ ; //v[idx].stillexist=true ;
     else
     {
        if (idx>0 && isdeleted (v[idx-1]))
            v[idx-1]=a ;
        else
            postpone_insertion (a, idx) ;
     }
    }
    return idx ;
}
//==============================================================*/
