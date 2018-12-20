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
            it->isghost=a.isghost ;
            it++ ;
        }
        else {it=v.insert(it,a) ; it++ ; }
    }
    return (cid++) ;
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
