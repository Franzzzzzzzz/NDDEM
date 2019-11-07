//#include "ContactList.h"





//=================================================================
// Sparse vector implementation, probably shouldn't use
//============================================================================================================
//------------------ Quite slow unfortunately. Probably better to use the list implementation ----------------
/*class sparsevector
{
public:
    sparsevector (int m) : max_size(m) {}
    void insert_in_place (vector <cp> & v1) ;
    int find_next_insertion (vector <cp> &v, cp &a) ;
    void mark_to_delete (vector <cp> & v) ;
    void finalise (vector<cp> &v) ;
    void reset(void) {idx=0 ;}
private:
    vector <cp> vin ;
    vector <int> location ;
    void postpone_insertion(cp & a, int idx) ;
    void compact (vector <cp> & v) ;
    int max_size ;
    int idx ;
    bool isdeleted (const cp &a) {return (a.i<0) ;}
} ;*/

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

//-------------------------------------- Recursive version of CheckGhosts, slow.
/*void ContactList::check_ghost (u_int32_t gst, int n, double partialsum, u_int32_t mask, const Parameters & P, cv1d &X1, cv1d &X2, double R, cp & tmpcp)
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
}*/

//------------------------------ A good version but the one above is faster
/*void ContactList::check_ghost (u_int32_t gst, double partialsum, const Parameters & P, cv1d &X1, cv1d &X2, double R, cp & tmpcp)
{
    double sum ; 
    
    double Delta ; 
    
    auto lbd = [&](double s, int m){
        //printf("\n {%X %X}\n\n", gstold, mask) ; fflush(stdout)  ; 
        //tmpcp.i=i ; tmpcp.j=j ; //must be set up before calling
        tmpcp.contactlength=sqrt(s) ;
        tmpcp.ghost=m ;
        //tmpcp.ghostdir must be set up before calling
        printf("2|%d %d %d \n", tmpcp.i, tmpcp.j, m) ;
        insert(tmpcp) ;
        //printf("CONTACT\n") ;
    } ;
    
    if (gst==0) return ; 
    
    int n=0 ; 
    for (int dd=0 ; gst ; gst>>=1, dd++ )
    {
     if (gst & 1) 
     {
      pbcdim[n] = dd ; 
      Delta= (tmpcp.ghostdir&(1<<dd)?-1:1) * P.Boundaries[dd][2] ;
      deltamap[n]= Delta * (2*(X2[dd]-X1[dd]) + Delta) ;
      masking[n] = 1<<dd ; 
      n++ ; 
     }
    }
    
    if (n==1) // Only 1 pbc, can be fast
    {
        sum = partialsum + deltamap[0] ;
        if (sum<R) lbd (sum, masking[0]) ; 
        return ; 
    }
    
    if (n==2) // Again, small enough to do everything manually, probably
    {
        sum = partialsum + deltamap[0] ;
        if (sum<R) lbd (sum, masking[0]) ; 
        
        sum = sum + deltamap[1] ;
        if (sum<R) lbd (sum, masking[0] | masking[1]) ; 
        
        sum=partialsum + deltamap[1] ;
        if (sum<R) lbd (sum, masking[1]) ; 
        
        return ; 
    }
    
    // Fallback case
    {
      int recur = 1<<n ; 
      for (int i=1 ; i<recur ; i++)
      {
        sum=partialsum ; 
        for (int j=0 ; j<n ; j++)
           if ((1<<j) & i)
             sum += deltamap[j] ; 
        if (sum<R) 
        {
            u_int32_t mask=0 ; 
            for (int j=0 ; j<n ; j++) {mask += ((i>>j)&1) * masking[j] ; }
            lbd (sum, mask) ;
        }
      }        
    }

} */
