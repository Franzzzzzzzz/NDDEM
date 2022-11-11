#ifndef TOOLS
#define TOOLS
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <vector>
#include <cmath>
#include "../Dem/Typedefs.h"
#include <boost/math/special_functions/factorials.hpp>
#include <boost/random.hpp>

template <int d>
class Tools
{
public:
    static void initialise ();
    static void clear() ;
    static void matvecmult (v1d & res, cv1d &A, cv1d &v);
    static double hyperspherical_xtophi (cv1d &x, v1d &phi);
    static void hyperspherical_phitox (double r, cv1d &phi, v1d &x) ;

    static double normsq (const vector <double> & a) {double res=0 ; for (int i=0 ; i<d ; i++) res+=a[i]*a[i] ; return (res) ; }
    static int sgn (uint8_t a) {return a & (128) ? -1:1 ; }
    static v1d transpose (cv1d & a) {v1d b (d*d,0) ; for (int i=0 ; i<d*d ; i++) b[(i/d)*d+i%d] = a[(i%d)*d+(i/d)] ; return b ; }
    static void transpose_inplace (v1d & a) { for (int i=0 ; i<d ; i++) for (int j=i+1 ; j<d ; j++) std::swap(a[i*d+j], a[j*d+i]) ; }

    static v1d Eye ;
    static boost::random::mt19937 rng ;
    static boost::random::uniform_01<boost::mt19937> rand ;

private:
    static vector < vector <int> > MSigns ;
    static vector < vector <int> > MIndexAS ;
    static vector < pair <int,int> > MASIndex ;
    static vector <FILE *> outs ;
};

// Static member definitions ---------------------------------------------------
template <int d> vector < vector <int> > Tools<d>::MSigns ;
template <int d> vector < vector <int> > Tools<d>::MIndexAS ;
template <int d> vector < pair <int,int> > Tools<d>::MASIndex;
template <int d> vector < double > Tools<d>::Eye ;
template <int d> vector <FILE *> Tools<d>::outs ;
template <int d> boost::random::mt19937 Tools<d>::rng ;
template <int d> boost::random::uniform_01<boost::mt19937> Tools<d>::rand(rng) ;

//--------------------------------------------------------------------------------
// All vector operators
inline v1d operator* (v1d a, double b)  {for (uint i=0 ; i<a.size() ; i++) a[i]*=b    ; return a ; }
inline v1f operator* (v1f a, float b)  {for (uint i=0 ; i<a.size() ; i++) a[i]*=b    ; return a ; }
inline v1d operator* (v1d a, cv1d &b)    {for (uint i=0 ; i<a.size() ; i++) a[i]*=b[i] ; return a ; }
inline v1f operator* (v1f a, cv1f &b)    {for (uint i=0 ; i<a.size() ; i++) a[i]*=b[i] ; return a ; }
inline v1d operator+ (v1d a, double b)  {for (uint i=0 ; i<a.size() ; i++) a[i]+=b    ; return a ; }
inline v1d operator+ (v1d a, cv1d &b)    {for (uint i=0 ; i<a.size() ; i++) a[i]+=b[i] ; return a ; }
inline v1f operator+ (v1f a, cv1f &b)    {for (uint i=0 ; i<a.size() ; i++) a[i]+=b[i] ; return a ; }
inline v1d operator- (v1d a, double &b)  {for (uint i=0 ; i<a.size() ; i++) a[i]-=b    ; return a ; }
inline v1d operator- (v1d a, cv1d &b)    {for (uint i=0 ; i<a.size() ; i++) a[i]-=b[i] ; return a ; }
inline v1d operator- (v1d a)            {for (uint i=0 ; i<a.size() ; i++) a[i]=-a[i] ; return a ; }
inline v1f operator- (v1f a, cv1f &b)    {for (uint i=0 ; i<a.size() ; i++) a[i]-=b[i] ; return a ; }
inline v1d operator/ (v1d a, double b)  {for (uint i=0 ; i<a.size() ; i++) a[i]/=b    ; return a ; }
inline v1d & operator-= (v1d & a, cv1d &b)  {for (uint i=0 ; i<a.size() ; i++) a[i]-=b[i] ; return a; }
inline v1d & operator*= (v1d & a, double b)  {for (uint i=0 ; i<a.size() ; i++) a[i]*=b ; return a; }
inline v1f & operator*= (v1f & a, double b)  {for (uint i=0 ; i<a.size() ; i++) a[i]*=b ; return a; }
inline v1d & operator+= (v1d & a, cv1d &b)  {for (uint i=0 ; i<a.size() ; i++) a[i]+=b[i] ; return a; }
inline v1f & operator+= (v1f & a, cv1f &b)  {for (uint i=0 ; i<a.size() ; i++) a[i]+=b[i] ; return a; }
inline v1f & operator/= (v1f & a, cv1f &b)  {for (uint i=0 ; i<a.size() ; i++) a[i]/=b[i] ; return a; }
inline v1d & operator/= (v1d & a, double b)  {for (uint i=0 ; i<a.size() ; i++) a[i]/=b ; return a; }
inline v1f & operator/= (v1f & a, double b)  {for (uint i=0 ; i<a.size() ; i++) a[i]/=b ; return a; }

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
void Tools<d>::initialise ()
{
 MSigns.resize(d, vector <int> (d, 0)) ;
 for (int i=0 ; i<d ; i++) for (int j=0 ; j<d ; j++) MSigns[i][j]=(i<j)*(1)+(i>j)*(-1) ;

 MIndexAS.resize(d, vector < int > (d,0)) ;
 MASIndex.resize(d*(d-1)/2, make_pair(0,0)) ;
 int n=0 ;
 for (int i=0 ; i<d ; i++)
      for (int j=i+1 ; j<d ; j++,n++)
      {
          MIndexAS[i][j]=n ; MIndexAS[j][i]=n ;
          MASIndex[n]=make_pair(i,j) ;
          //MASIndex.push_back(make_pair(i, j)) ;
      }

  Eye.clear() ; Eye.resize(d*d,0) ;
  for (int de=0 ; de<d ; de++) Eye[de*d+de]=1 ; //initial orientation matrix
}
//===================================
template <int d>
void Tools<d>::clear()
{
    MSigns.clear() ;
    MIndexAS.clear() ;
    MASIndex.clear() ;
    Eye.clear() ;
}
//-------------------------------
template <int d>
void Tools<d>::matvecmult (v1d & res, cv1d &A, cv1d &v)
{
    for (int i=0 ; i<d ; i++) res[i] = 0.0;
    for (int i=0 ; i<d; i++)
        for (int k=0 ; k<d ; k++)
            res[i]+=A[i*d+k]*v[k] ;
}
//--------------------------------
template <int d>
double Tools<d>::hyperspherical_xtophi (cv1d &x, v1d &phi) // WARNING NOT EXTENSIVELY TESTED
{
    double rsqr = normsq(x) ; // norm of x_rotated
    double r= sqrt(rsqr) ;
    int j;
    phi=vector<double>(d-1, 0) ;
    for (j=d-1 ; j>=0 && fabs(x[j])<1e-6 ; j--) ;
    int lastnonzero=j ;
    for (j=0 ; j<d-1 ; j++)
    {
       if (j==lastnonzero)
       {
           if (x[j]<0) phi[j]=M_PI ;
           else phi[j]=0 ;
           return r ;
       }
       phi[j] = acos(x[j]/sqrt(rsqr)) ;
       //printf("[%g %g]", x[j], sqrt(rsqr)) ;
       if (isnan(phi[j])) {phi[j]=acos(sgn(x[j])*x[j]) ;} //TODO Check that ................
       rsqr -= x[j]*x[j] ;
    }
    //printf("%g %g %g | %g | %g %g \n ", x[0], x[1], x[2], normsq(x), phi[0], phi[1]) ;
    if (x[d-1]<0) phi[d-2] = 2*M_PI - phi[d-2] ;
    return r ;
}

template <int d>
void Tools<d>::hyperspherical_phitox (double r, cv1d &phi, v1d &x) // WARNING NOT EXTENSIVELY TESTED
{
    x = v1d (d,r) ;
    for (int i=0 ; i<d-1 ; i++)
    {
        x[i] *= cos(phi[i]) ;
        for (int j=i+1 ; j<d ; j++)
            x[j] *= sin(phi[i]) ;
    }
    x[d-1] *= sin(phi[d-2]) ;
}
#endif
