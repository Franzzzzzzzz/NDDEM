/** \addtogroup DEM Discrete Element Simulations
 * This module handles the Discrete Element Simulations.
 *  @{ */

#ifndef TOOLS
#define TOOLS
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <vector>
#include <cmath>
#include <initializer_list>
#include "Typedefs.h"
#include "Parameters.h"


#include <boost/math/special_functions/factorials.hpp>
#include <boost/random.hpp>

#define MAXDEFDIM 30 ///< For larger number of dimension,  be taken in particular for periodic boundary conditions


v1d operator* (v1d a, double b) ;
v1f operator* (v1f a, float b) ;
v1d operator* (v1d a, cv1d &b)   ;
v1f operator* (v1f a, cv1f &b)  ;
v1d operator+ (v1d a, double b) ;
v1d operator+ (v1d a, cv1d &b)   ;
v1f operator+ (v1f a, cv1f &b)   ;
v1d operator- (v1d a, double b) ;
v1d operator- (v1d a, cv1d &b)   ;
v1f operator- (v1f a, cv1f &b)   ;
v1d operator- (v1d a, const double * b) ;
v1d operator- (const double * a, v1d b) ;
v1d operator- (v1d a) ;
v1d operator/ (v1d a, double b) ;
v1d & operator-= (v1d & a, cv1d &b) ;
v1d & operator*= (v1d & a, double b);
v1f & operator*= (v1f & a, double b);
v1d & operator+= (v1d & a, cv1d &b) ;
v1f & operator+= (v1f & a, cv1f &b) ;
v1f & operator/= (v1f & a, cv1f &b) ;
v1d & operator/= (v1d & a, double b) ;
v1f & operator/= (v1f & a, double b) ;

enum class TensorType {SCALAR, VECTOR, TENSOR, SYMTENSOR, SKEWTENSOR, SCALARMASK, NONE} ; ///< Different types of mathematical objects
/** \brief Limited use: used to transfer data to the VTK writer
 */
class TensorInfos
{
public :
  string name ;
  TensorType order ;
  v2d * data ;
} ;


/** \brief Dimension specific mathematics */
class Tools_2D
{
public:
  static std::tuple<double,double> contact_ellipse_disk (std::vector<double> & X, 
                                                  double a, double b, double cx, double cy, //Ellipse parameters
                                                  double gamma=0.1, double tol=1e-5)
  {
  auto d0=[&](double u){return sqrt((X[0]-cx-a*cos(u))*(X[0]-cx-a*cos(u))+(X[1]-cy-b*sin(u))*(X[1]-cy-b*sin(u)));} ; 
  auto d1=[&](double u){return (2*a*(X[0]-cx-a*cos(u))*sin(u)-2*b*cos(u)*(X[1]-cy-b*sin(u)));} ; 
  //d2=@(u) (-(-b*cos(u))*(2*b*cos(u))+cos(u)*(2*a*(x-cx-a*cos(u)))+(a*sin(u))*(2*a*sin(u))--sin(u)*(2*b*(y-cy-b*sin(u)))) ;

  //Gradient descent 
  int n=0 ; double delta=1 ;
  double to=atan2(X[1]-cy,X[0]-cx), tn ;
  while (delta>tol) 
  {
    tn= to - gamma * d1(to) ; 
    delta=fabs(tn-to) ; 
    to=tn ;
    n++ ; 
    if (n>1000) {printf("ERR: maximum number of iteration reached in gradient descent. gradientdescent_gamma is probably too large\n") ; break ;}
  }
  
  return {to, d0(to)} ; 
  }
  
} ; 



/** \brief Static class to handle multi-dimensional mathematics, and more. It gets specialised for speed with template parameter d:dimension
 */

/**  \par Matrix storage
 *   <UL>
 *   <LI> Arbitrary square matrix are stored as linear array, in the order x11, x12, x13 ... x21, x22 ... xdd. They have d^2 components. </LI>
 *   <LI> Skew-symetric matrix only store their top right side, in the order x12, x13, x14 ... x23, x24 ... x(d-1)d. They have d(d-1)/2 components. These are handled using the specific function skewmatvecmult() and similar, and make use of the special variables #MSigns, #MIndexAS and #MASIndex </LI>
 *   <LI> Symetric matrix are not really needed anywhere, but would store only the top right side + diagonal as a linear array, in the order x11, x12 ... x22, x23, ... xdd. They have d(d+1)/2 components.  </LI>
 *   </UL>
 * */
template <int d>
class Tools
{
public:
/** @name General functions */
///@{
static void initialise () ; ///< Initialise the member variables, in particular the variables to handle skew-symmetric flattened matrix, cf. the class detailed description.
static void clear() ; ///< Get the class ready for a different dimension.
static bool check_initialised (int dd) {return (dd==d) ; }
static int sgn (uint8_t a) {return a & (128) ? -1:1 ; } ///< Sign function
static int sgn (double a)   {return a<0 ? -1:1 ; } ///< Sign function
static std::pair <double, double> two_max_element (cv1d & v) ; ///< Return the two largest elements of v
///@}

/** @name Vector and matrix initialisation and normalisation */
///@{
static void unitvec (vector <double> & v, int n) ;                    ///< Construct a unit vector in place
static v1d unitvec (int n) {v1d res (d,0) ; res[n]=1 ; return res ; } ///< Construct & return a unit vector
static void setzero(v2d & a) {for (uint i=0 ; i<a.size() ; i++) for (uint j=0 ; j<a[0].size() ; j++) a[i][j]=0 ; } ///< Set a matrix to zero in-place
static void setzero(v1d & a) {for (uint i=0 ; i<a.size() ; i++) a[i]=0 ; } ///< Set a vector to zero in-place
static double norm (const vector <double> & a) {double res=0 ; for (uint i=0 ; i<a.size() ; i++) res+=a[i]*a[i] ; return (sqrt(res)) ; } ///< Norm of a vector
static   void norm (v1d & res, cv2d & a) {for (uint i=0 ; i<a.size() ; i++) res[i] = norm(a[i]) ; } ///< Norm of a 2D-matrix, returns a 1D vector with the norms of the individual vectors
static double normdiff (cv1d & a, cv1d & b) {double res=0 ; for (int i=0 ; i<d ; i++) res+=(a[i]-b[i])*(a[i]-b[i]) ; return (sqrt(res)) ; } ///< Norm of a vector difference \f$|a-b|\f$
static double normsq (const vector <double> & a) {double res=0 ; for (int i=0 ; i<d ; i++) res+=a[i]*a[i] ; return (res) ; } ///< Norm squared
static double normdiffsq (cv1d & a, cv1d & b) {double res=0 ; for (int i=0 ; i<d ; i++) res+=(a[i]-b[i])*(a[i]-b[i]) ; return (res) ; } ///< Norm squared of a vector difference \f$|a-b|^2\f$
static void orthonormalise (v1d & A) ; ///< Orthonormalise A using the Gram-Shmidt process, in place
static double skewnorm (cv1d & a) {double res=0 ; for (int i=0 ; i<d*(d-1)/2 ; i++) res+=a[i]*a[i] ; return (sqrt(res)) ; } ///<Norm of a skew-symetric matrix
static double skewnormsq (cv1d & a) {double res=0 ; for (int i=0 ; i<d*(d-1)/2 ; i++) res+=a[i]*a[i] ; return (res) ; } ///< Norm squared of a skew-symetrix matrix
static double dot (cv1d & a, cv1d & b) {double res=0; for (int i=0 ; i<d ; i++) res+=a[i]*b[i] ; return (res) ; } ///< Dot product
static v1f vsqrt (cv1f & a) {v1f b=a ; for (uint i=0 ; i<a.size() ; i++) b[i]=sqrt(a[i]) ; return b ; } ///<Component-wise square root
static v1f vsq (cv1f & a) {v1f b=a ; for (uint i=0 ; i<a.size() ; i++) b[i]=a[i]*a[i] ; return b ; } ///< Component-wise squaring

static void surfacevelocity (v1d &res, cv1d &p, double * com, double * vel_com, double * omega_com)
{
 if (vel_com == nullptr && omega_com == nullptr)
 {
     res.resize(d) ; setzero(res) ;
 }
 else if (omega_com == nullptr)
     for (int dd=0 ; dd<d ; dd++)
         res[dd] = vel_com[dd] ;
 else if (vel_com == nullptr)
 {
    skewmatvecmult(res, omega_com, p-com) ;
    //printf("{%g %g %g %g %g}", omega_com[0], (p-com)[0], (p-com)[1], res[0], res[1]) ;
 }
 else
 {
     skewmatvecmult(res, omega_com, p-com) ;
     for (int dd=0 ; dd<d ; dd++)
        res[dd] = vel_com[dd] - res[dd] ;
 }
}


static void setgravity(v2d & a, v1d &g, v1d &m) {for (uint i=0 ; i<a.size() ; i++) a[i]=g*m[i] ; } ///< Set the gravity. \f$\vec a_i = m_i * \vec g \f$
static v1d randomize_vec (cv1d v) ; ///< Produce a random vector
///@}

/** @name Coordinate system change */
///@{
static double hyperspherical_xtophi (cv1d &x, v1d &phi) ; ///< Convert from cartesian to hyperspherical coordinates
static void   hyperspherical_phitox (double r, cv1d &phi, v1d &x) ; ///< Convert from hyperspherical to cartesian coordinates
///@}

/** @name Special physics computation */
///@{
static double Volume (double R) ; ///< Compute the hypersphere volume
static double InertiaMomentum (double R, double rho) ; ///< Compute the hypersphere moment of inertia
///@}


/** @name Faster vector operations
 *  These vector or matrix operations are working in place, and sometimes perform multiple operations at once
 */
///@{
static void vMul  (v1d & res, cv1d &a, double b)  {for (uint i=0 ; i<a.size() ; i++) res[i]=a[i]*b ; }    ///< Multiply a vector by a scalar in-place
static void vMul  (v1d & res, cv1d &a, cv1d & b)  {for (uint i=0 ; i<a.size() ; i++) res[i]=a[i]*b[i] ; } ///< Component-wise multiply 2 vectors in-place
static void vMinus (v1d & res, cv1d &a, cv1d & b) {for (uint i=0 ; i<a.size() ; i++) res[i]=a[i]-b[i] ; } ///< Difference of 2 vectors in-place \f$a-b\f$
static void vPlus (v1d & res, cv1d &a, double b)  {for (uint i=0 ; i<a.size() ; i++) res[i]=a[i]+b ; }    ///< Addition of a vector by a scalar in-place
static void vPlus (v1d & res, cv1d &a, cv1d & b)  {for (uint i=0 ; i<a.size() ; i++) res[i]=a[i]+b[i] ; } ///< Addition of 2 vectors in-place
static void vDiv   (v1d & res, cv1d &a, double b) {for (uint i=0 ; i<a.size() ; i++) res[i]=a[i]/b ; }    ///< Division of a vector by a scalar in-place
static void vDiv   (v1d & res, cv1d &a, cv1d & b) {for (uint i=0 ; i<a.size() ; i++) res[i]=a[i]/b[i] ; } ///< Component-wise discrete of 2 vectors in-place
static void vAddFew (v1d &res , cv1d &a, cv1d &b)    {for (uint i=0 ; i<a.size() ; i++) res[i] += a[i]+b[i] ; } /// Addition of 3 vectors in-place
static void vAddScaled (v1d &res , double v, cv1d &a, cv1d &b) {for (uint i=0 ; i<a.size() ; i++) res[i] += v*(a[i]+b[i]) ; } ///< Addition of two scaled vector \f$ res := res + v*(a+b)\f$
static void vAddScaled (v1d &res , double v, cv1d &a)          {for (uint i=0 ; i<a.size() ; i++) res[i] += v*a[i] ; } ///< Addition of a scaled vector \f$ res := res + v*a\f$
static void vSubFew (v1d &res , cv1d &a, cv1d &b)                 {for (uint i=0 ; i<a.size() ; i++) res[i] -= (a[i]+b[i]) ; } ///< Subtraction of 2 vectors \f$ res := res - a - b\f$
static void vSubScaled (v1d &res , double v, cv1d &a)          {for (uint i=0 ; i<a.size() ; i++) res[i] -= v*a[i] ; } ///< Subtraction of a scaled vector \f$ res := res - v*a\f$
///@}

/** @name Faster vector operations with error corrections (Kahan summation algorithm)
 *  These vector or matrix operations are working in place, perform multiple operations at once, and are designed to use error correction across multiple calls.
 */
///@{
static void vAddFew (v1d & res, cv1d &a, cv1d &b, v1d & Corr)
{
  double Tmp, Previous ;

  for (uint i=0 ; i<a.size() ; i++)
  {
  Tmp=(a[i]+b[i])-Corr[i] ;
  Previous=res[i] ;
  res[i] += Tmp ;
  Corr[i] = (res[i]-Previous)-Tmp ;
  }
} ///< Addition of 3 vectors in-place with error correction (Kahan summation algorithm)
static void vSubFew (v1d & res, cv1d &a, cv1d &b, v1d & Corr)
{
  double Tmp, Previous ;

  for (uint i=0 ; i<a.size() ; i++)
  {
  Tmp=(-a[i]-b[i])-Corr[i] ;
  Previous=res[i] ;
  res[i] += Tmp ;
  Corr[i] = (res[i]-Previous)-Tmp ;
  }
} ///< Subtraction of 2 vectors \f$ res := res - a - b\f$ with error correction
static void vAddOne (v1d & res, cv1d &a, v1d & Corr)
{
  double Tmp, Previous ;

  for (uint i=0 ; i<a.size() ; i++)
  {
  Tmp=a[i]-Corr[i] ;
  Previous=res[i] ;
  res[i] += Tmp ;
  Corr[i] = (res[i]-Previous)-Tmp ;
  }
} ///< Addition of 2 vectors in-place with error correction (Kahan summation algorithm)
static void vSubOne (v1d & res, cv1d &a, v1d & Corr)
{
  double Tmp, Previous ;

  for (uint i=0 ; i<a.size() ; i++)
  {
  Tmp=-a[i]-Corr[i] ;
  Previous=res[i] ;
  res[i] += Tmp ;
  Corr[i] = (res[i]-Previous)-Tmp ;
  }
}///< Subtraction of 2 vectors in-place with error correction (Kahan summation algorithm)
///@}

/** @name Matrix operations, usually operating on flattened matrices, cf. the description for more information on matrix storage */
///@{
static v1d  skewmatvecmult (cv1d & M, cv1d &v) ; ///< Multiply the skew symetric matrix M with vector v
static v1d  skewmatvecmult (const double * M, cv1d &v) ; ///< Multiply the skew symetric matrix M with vector v
static void skewmatvecmult (v1d & r, cv1d & M, cv1d &v) ; ///< Multiply the skew symetric matrix M with vector v in place
static void skewmatvecmult (v1d & r, const double * M, cv1d &v) ; ///< Multiply the skew symetric matrix M with vector v in place (overload)
static v1d  skewmatsquare  (cv1d &A) ;///< Square the skew symetric matrix M
static void skewmatsquare  (v1d &r, cv1d &A) ; ///< Square the skew symetric matrix M in place
static v1d  skewexpand     (cv1d &A) ; ///< Return the skew symetrix matrix M stored on d(d-1)/2 component as a full flattened matrix with d^2 components
static void skewexpand     (v1d & r, cv1d &A) ; ///< Return the skew symetrix matrix M stored on d(d-1)/2 component as a full flattened matrix with d^2 components in place
static v1d  matmult (cv1d &A, cv1d &B) ; ///< Multiply 2 matrix together
static void matmult (v1d &r, cv1d &A, cv1d &B) ; ///< Multiply 2 matrix together in place
static void  matvecmult (v1d & res, cv1d &A, cv1d &B) ; ///< Multiply a matrix with a vector, in place.
static v1d  wedgeproduct (cv1d &a, cv1d &b) ; ///< Wedge product of vectors
static void wedgeproduct (v1d &res, cv1d &a, cv1d &b) ; ///< Wedge product in-place
static v1d transpose (cv1d & a) {v1d b (d*d,0) ; for (int i=0 ; i<d*d ; i++) b[(i/d)*d+i%d] = a[(i%d)*d+(i/d)] ; return b ; } ///< Transposition
static void transpose_inplace (v1d & a) { for (int i=0 ; i<d ; i++) for (int j=i+1 ; j<d ; j++) std::swap(a[i*d+j], a[j*d+i]) ; } ///< Transpose in-place
static double det (cv2d & M) ; ///< compute the matrix determinant (probably quite slow, but doesn't really really matters for the usage)
static double det (cv1d & M) ; ///< compute the matrix determinant (probably quite slow, but shouldn't really really matters for the usage)
static v1d inverse (cv1d & M) ; ///< compute the matrix inverse (very slow and redundant calculation of the determinant for the comatrix, but shouldn't really really matters for the usage)
///@}

/** @name Saving and writing functions */
///@{
static int savetxt(char path[], const v2d & table, char const header[]) ;
static void savecsv (char path[], cv2d & X, cv1d &r, const vector <uint32_t> & PBCFlags, cv1d & Vmag, cv1d & OmegaMag, [[maybe_unused]] cv1d & Z) ; ///< Save the location and a few more informations in a CSV file.
static void savecsv (char path[], cv2d & X, cv2d & V, cv1d &r, const vector <uint32_t> & PBCFlags, cv1d & Vmag, cv1d & OmegaMag, [[maybe_unused]] cv1d & Z) ; ///< Save the location and a few more informations in a CSV file.
static void savecsv (char path[], cv2d & A) ; ///< Save the orientation in a CSV file
static void savevtk (char path[], int N, cv2d & Boundaries, cv2d & X, cv1d & r, vector <TensorInfos> data) ; ///< Save as a vtk file. Dimensions higher than 3 are stored as additional scalars. Additional informations can be passed as a vector of #TensorInfos.

static void print (cv1d M) {printf("[") ; if (M.size()==d*d) for (int i=0 ; i<d ; i++) { for (int j=0 ; j<d ; j++) printf("%g ", M[i*d+j]) ; printf("\n") ; } else for (auto v: M) printf("%g ", v) ; printf("]") ; }

static int write1D (char path[], v1d table) ;
static int writeinline(initializer_list< v1d >) ;
static int writeinline_close(void) ;
///@}

static v1d Eye ; ///< The identity matrix in dimension \<d\>
static boost::random::mt19937 rng ; ///< Random number generator
static boost::random::uniform_01<boost::mt19937> rand ; ///< Returns a random number between 0 and 1

static int getdim (void) {return d;} ///< Return the dimension. \deprecated{Probably deprecated: essentially so that the NetCDF class has access to d.}

static vector < pair <int,int> > MASIndex ; ///< For skew symmetric matrix, make the correspondance between linear index and (row,column) index.

private:
static vector < vector <int> > MSigns ; ///< For skew symetric matrix. -1 below the diagonal, 0 on the diagonal, +1 above the diagnal
static vector < vector <int> > MIndexAS ; ///< For skew symmetric matrix, make the correspondance between linear index of a full matrix with the linear index of the skew-symetric storage.
static vector <FILE *> outs ; ///< Store the output file descriptors.

} ;

// Static member definitions ---------------------------------------------------
template <int d> vector < vector <int> > Tools<d>::MSigns ;
template <int d> vector < vector <int> > Tools<d>::MIndexAS ;
template <int d> vector < pair <int,int> > Tools<d>::MASIndex;
template <int d> vector < double > Tools<d>::Eye ;
template <int d> vector <FILE *> Tools<d>::outs ;
template <int d> boost::random::mt19937 Tools<d>::rng ;
template <int d> boost::random::uniform_01<boost::mt19937> Tools<d>::rand(rng) ;


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

//===================================
template <int d>
int Tools<d>::savetxt(char path[], const v2d & table, char const header[])
{
 FILE * out ;
 out = fopen(path, "w") ; if (out==NULL) {printf("ERR: cannot write in file %s\n", path) ; return 1 ; }
 fprintf(out, "%s\n", header) ;
 for (uint i=0 ; i<table.size() ; i++)
 {
     for (uint j=0 ; j<table[i].size() ; j++)
     {
         fprintf(out, "%.6g",table[i][j]) ;
         if (j<table[i].size()-1) fprintf(out, ",") ;
     }
     if (i<table.size()-1) fprintf(out, "\n") ;
 }
 fclose(out) ;
 return 0 ;
}
//=======================================
template <int d>
int Tools<d>::write1D (char path[], v1d table)
{
 FILE * out ;
 out = fopen(path, "w") ; if (out==NULL) {printf("ERR: cannot write in file %s\n", path) ; return 1 ; }
 for (uint i=0 ; i<table.size() ; i++)
   fprintf(out, "%g\n", table[i]) ;
 fclose(out) ;
return 0 ;
}

//=====================================
template <int d>
void Tools<d>::savecsv (char path[], cv2d & X, cv1d &r, const vector <uint32_t> & PBCFlags, cv1d & Vmag, cv1d & OmegaMag, [[maybe_unused]] cv1d & Z )
{
 FILE *out ; int dim ;
 out=fopen(path, "w") ; if (out==NULL) {printf("Cannot open out file\n") ; return ;}
 dim = X[0].size() ;
 for (int i=0 ; i<dim ; i++) fprintf(out, "x%d,", i);
 fprintf(out, "R,PBCFlags,Vmag,Omegamag\n") ;
 for (uint i=0 ; i<X.size() ; i++)
 {
  for (int j=0 ; j<dim ; j++)
    fprintf(out, "%.6g,", X[i][j]) ;
  fprintf(out, "%g,%d,%g,%g\n", r[i],PBCFlags[i], Vmag[i], OmegaMag[i]) ;
 }
 fclose(out) ;
}
//=====================================
template <int d>
void Tools<d>::savecsv (char path[], cv2d & X, cv2d & V, cv1d &r, const vector <uint32_t> & PBCFlags, cv1d & Vmag, cv1d & OmegaMag, [[maybe_unused]] cv1d & Z )
{
 FILE *out ; int dim ;
 out=fopen(path, "w") ; if (out==NULL) {printf("Cannot open out file\n") ; return ;}
 dim = X[0].size() ;
 for (int i=0 ; i<dim ; i++) fprintf(out, "x%d,", i);
 for (int i=0 ; i<dim ; i++) fprintf(out, "v%d,", i);
 fprintf(out, "R,PBCFlags,Vmag,Omegamag\n") ;
 for (uint i=0 ; i<X.size() ; i++)
 {
  for (int j=0 ; j<dim ; j++) fprintf(out, "%.6g,", X[i][j]) ;
  for (int j=0 ; j<dim ; j++) fprintf(out, "%.6g,", V[i][j]) ;
  fprintf(out, "%g,%d,%g,%g\n", r[i],PBCFlags[i], Vmag[i], OmegaMag[i]) ;
 }
 fclose(out) ;
}
//-----------------------------------------
template <int d>
void Tools<d>::savecsv (char path[], cv2d & A)
{
 FILE *out ; int dim ;
 out=fopen(path, "w") ; if (out==NULL) {printf("Cannot open out file\n") ; return ;}
 dim = A[0].size() ;
 for (int i=0 ; i<dim-1 ; i++) fprintf(out, "x%d,", i);
 fprintf(out, "x%d\n", dim-1) ;
 for (uint i=0 ; i<A.size() ; i++)
 {
  for (int j=0 ; j<dim-1 ; j++)
    fprintf(out, "%.6g,", A[i][j]) ;
  fprintf(out, "%.6g", A[i][dim-1]) ;
  fprintf(out, "\n") ;
 }
 fclose(out) ;
}
//=====================================
template <int d>
void Tools<d>::savevtk (char path[], int N, cv2d & Boundaries, cv2d & X, cv1d & r, vector <TensorInfos> data)
{
 FILE *out ; static bool warn = false ;

 vector <float> projectioncenter  ; for (int i=3 ; i<d ; i++) projectioncenter.push_back((Boundaries[i][1]+Boundaries[i][0])/2) ;

 out=fopen(path, "w") ; if (out==NULL) {printf("Cannot open out file\n") ; return ;}

 if (d>3 && warn==false) {
   printf("WARN: writevtk might misbehave with dimension higher than 3. The 3d projection is always centered in all other dimensions\n") ;
   warn=true ;
 }
 fprintf(out, "# vtk DataFile Version 2.0\nMost Useless DEM (tm) output file\nASCII\nDATASET POLYDATA\n") ;

 fprintf(out, "POINTS %ld float\n", X.size()) ;
 for (uint i=0 ; i<X.size() ; i++) fprintf(out, "%g %g %g\n", X[i][0], X[i][1], d<3?0:X[i][2]) ;
 fprintf(out, "VERTICES %ld %ld\n", X.size(), 2*X.size()) ;
 for (uint i=0 ; i<X.size() ; i++) fprintf(out, "1 %d\n", i) ;

 fprintf(out, "\nPOINT_DATA %ld", X.size()) ;

 for (uint j=3 ; j<X[0].size() ; j++)
 {
   fprintf(out, "\nSCALARS Dimension%d float 1 \nLOOKUP_TABLE default \n", j) ;
   for (uint i=0 ; i<X.size() ; i++)
       fprintf(out, "%g ", X[i][j]) ;
 }

 fprintf(out, "\n\nSCALARS RadiusProjected float 1 \nLOOKUP_TABLE default\n");
 for (int i=0 ; i<N ; i++)
 {
   float value = r[i]*r[i] ;
   for (int j=3 ; j<d ; j++) value-=(X[i][j]-projectioncenter[j-3])*(X[i][j]-projectioncenter[j-3]) ;
   if (value<0) fprintf(out, "%g ", 0.0) ;
   else fprintf(out, "%g ", sqrt(value)) ;
 }

 for (auto v : data)
 {
 switch (v.order) {
   case TensorType::SCALAR:  fprintf(out, "\nSCALARS %s double 1 \nLOOKUP_TABLE default \n", v.name.c_str()) ;//scalar
            for (uint i=0 ; i<(*v.data)[0].size() ; i++)
              fprintf(out, "%g ", (*v.data)[0][i]) ;
            break ;
   case TensorType::VECTOR:  fprintf(out, "\nVECTORS %s double \n", v.name.c_str()) ;//vector
            for (auto i : (*v.data))
              fprintf(out, "%g %g %g\n", i[0], i[1], i[2]) ;
            break ;
   case TensorType::TENSOR:  fprintf(out, "\nTENSORS %s double \n", v.name.c_str()) ;//tensor
            for (auto i : (*v.data))
              fprintf(out, "%g %g %g %g %g %g %g %g %g\n", i[0], i[1], i[2], i[d], i[d+1], i[d+2], i[2*d], i[2*d+1], i[2*d+2]) ;
            break ;
   case TensorType::SYMTENSOR:  fprintf(out, "\nTENSORS %ssym double \n", v.name.c_str()) ;//tensor
            for (auto i : (*v.data))
              fprintf(out, "%g %g %g %g %g %g %g %g %g\n", i[0], i[1], i[2], i[1], i[d], i[d+1], i[2], i[d+1], i[2*d-1]) ;
            break ;
   case TensorType::SKEWTENSOR:  fprintf(out, "\nTENSORS %sskew double \n", v.name.c_str()) ;//tensor
             for (v1d i : (*v.data))
               fprintf(out, "%g %g %g %g %g %g %g %g %g\n", 0.0, i[0], i[1], -i[0], 0.0, i[d-1], -i[1], -i[d-1], 0.0) ;
            break ;
   default: break ; /*fprintf(out, "\nPOINT_DATA %ld\nSCALARS %s double 1 \nLOOKUP_TABLE default \n",(*data.data).size(), data.name.c_str()) ;//scalar norm
              for (uint i=0 ; i<(*data.data).size() ; i++)
                 fprintf(out, "%g ", Tools<d>::norm((*data.data)[i])) ;*/
 }
 }



 fclose(out) ;
}
//============================================
template <int d>
int Tools<d>::writeinline(initializer_list< v1d > list)
{
  if (outs.size()==0)
  {
    char path[5000] ;
    outs.resize(list.size(), NULL) ;
    uint i=0 ;
    for (auto iter=list.begin() ; iter<list.end() ; iter++, i++)
    {
      sprintf(path, "Res-%d.txt", i) ;
      outs[i]=fopen(path, "w") ;
      if (outs[i]==NULL) {printf("Error cannot open writeinline file %d", i) ; return (1) ; }
    }
  }

  int i=0 ;
  for (auto iter=list.begin() ; iter<list.end() ; iter++, i++)
  {
    for (uint j=0 ; j<iter->size() ; j++ )
      fprintf(outs[i], "%g ", iter->at(j)) ;
    fprintf(outs[i], "\n") ;
  }

  return 0 ;
}

template <int d>
int Tools<d>::writeinline_close(void)
{
  for (uint i=0 ; i<outs.size() ; i++)
    fclose(outs[i]) ;
  return 0 ;
}
//=========================================
template <int d>
v1d Tools<d>::randomize_vec (cv1d v)
{
  v1d res ; res.resize(v.size(),0) ;
  for (uint i=0 ; i<res.size() ; i++) res[i]=rand() ;
  double n=Tools<d>::norm(v) ; double nr=Tools<d>::norm(res) ;
  for (uint i=0 ; i<res.size() ; i++) res[i]=res[i]/nr*n ;
  return (res) ;
}
//=============================================
template <int d>
std::pair <double, double> Tools<d>::two_max_element (cv1d & v)
{
  double m1=v[0],m2=v[1] ;
  if (m1<m2) {double tmp ; tmp=m1 ; m1=m2 ; m2=tmp ;} ;

  for (uint i=2 ; i<v.size() ; i++)
  {
    if (v[i] > m1) {m1=v[i] ; continue ; }
    if (v[i] > m2) {m2=v[i] ; }
  }
  return (make_pair(m1,m2)) ;
}

//=========================================

//-----------------------------------
//void Tools<d>::vec2eigen (Eigen::MatrixXd & a, cv1d &b) {for (uint i=0 ; i<d*d ; i++) a(i/3,i%3) = b[i] ; }
//void Tools<d>::eigen2vec (v1d &b, const Eigen::MatrixXd & a) {for (uint i=0 ; i<d*d ; i++) b[i] = a(i/3,i%3);}
template <int d>
v1d Tools<d>::skewmatvecmult (cv1d & M, cv1d & v)
{
 v1d res (d,0) ;
 for (int i=0 ; i<d ; i++)
 {
     for (int j=0 ; j<d ; j++)
     {
         if (j==i) continue ;
         res[i]+= MSigns[i][j]*M[MIndexAS[i][j]]*v[j] ;
     }
 }
 return res ;
}
template <int d>
v1d Tools<d>::skewmatvecmult (const double * M, cv1d & v)
{
 v1d res (d,0) ;
 for (int i=0 ; i<d ; i++)
 {
     for (int j=0 ; j<d ; j++)
     {
         if (j==i) continue ;
         res[i]+= MSigns[i][j]*M[MIndexAS[i][j]]*v[j] ;
     }
 }
 return res ;
}
//------------------------------------
template <int d>
void Tools<d>::skewmatvecmult (v1d & r, cv1d & M, cv1d & v)
{
 for (int i=0 ; i<d ; i++)
 {
     r[i]=0 ;
     for (int j=0 ; j<d ; j++)
     {
         if (j==i) continue ;
         r[i]+= MSigns[i][j]*M[MIndexAS[i][j]]*v[j] ;
     }
 }
}
template <int d>
void Tools<d>::skewmatvecmult (v1d & r, const double * M, cv1d & v)
{
 for (int i=0 ; i<d ; i++)
 {
     r[i]=0 ;
     for (int j=0 ; j<d ; j++)
     {
         if (j==i) continue ;
         r[i]+= MSigns[i][j]*M[MIndexAS[i][j]]*v[j] ;
     }
 }
}
//------------------------------------
template <int d>
v1d Tools<d>::skewmatsquare(cv1d & A)
{
    v1d res (d*d, 0) ;
    for (int i=0 ; i<d ; i++)
        for (int j=i ; j<d ; j++)
        {
         for (int k=0 ; k<d ; k++)
             res[i*d+j]+=A[MIndexAS[i][k]]*A[MIndexAS[k][j]]*MSigns[i][k]*MSigns[k][j] ;
        }

    for (int i=1 ; i<d ; i++)
        for (int j=0 ; j<i ; j++)
            res[i*d+j]=res[j*d+i] ;

    return res ;
}
//-------------------------------------
template <int d>
void Tools<d>::skewmatsquare(v1d & r, cv1d & A)
{
    for (int i=0 ; i<d ; i++)
        for (int j=i ; j<d ; j++)
        {
         r[i*d+j]=0 ;
         for (int k=0 ; k<d ; k++)
             r[i*d+j]+=A[MIndexAS[i][k]]*A[MIndexAS[k][j]]*MSigns[i][k]*MSigns[k][j] ;
        }

    for (int i=1 ; i<d ; i++)
        for (int j=0 ; j<i ; j++)
            r[i*d+j]=r[j*d+i] ;

}
//-------------------------------
template <int d>
v1d Tools<d>::skewexpand(cv1d & A)
{
    v1d res (d*d, 0) ;
    for (int i=0 ; i<d*d; i++)
    {
        res[i]=A[MIndexAS[i/d][i%d]]*MSigns[i/d][i%d] ;
    }
    return res ;
}
//-------------------------------
template <int d>
void Tools<d>::skewexpand(v1d & r, cv1d & A)
{
    for (int i=0 ; i<d*d; i++)
    {
        r[i]=A[MIndexAS[i/d][i%d]]*MSigns[i/d][i%d] ;
    }
}
//-------------------------------
template <int d>
v1d Tools<d>::matmult (cv1d &A, cv1d &B)
{
    v1d res (d*d, 0) ;
    for (int i=0 ; i<d; i++)
        for (int j=0 ; j<d ; j++)
            for (int k=0 ; k<d ; k++)
                res[i*d+j]+=A[i*d+k]*B[k*d+j] ;
    return res ;
}
//-------------------------------
template <int d>
void Tools<d>::matmult (v1d & r, cv1d &A, cv1d &B)
{
    setzero(r) ;
    for (int i=0 ; i<d; i++)
        for (int j=0 ; j<d ; j++)
            for (int k=0 ; k<d ; k++)
                r[i*d+j]+=A[i*d+k]*B[k*d+j] ;
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
//----------------------------------------
template <int d>
v1d Tools<d>::wedgeproduct (cv1d &a, cv1d &b)
{
  v1d res (d*(d-1)/2, 0) ; int k ;
  auto iter = MASIndex.begin() ;
  for (k=0 ; iter!= MASIndex.end() ; iter++, k++)
      res[k]=a[iter->first]*b[iter->second]-a[iter->second]*b[iter->first] ;
  return (res) ;
}
//----------------------------------------
template <int d>
void Tools<d>::wedgeproduct (v1d &res, cv1d &a, cv1d &b)
{
  int k ;
  auto iter = MASIndex.begin() ;
  for (k=0 ; iter!= MASIndex.end() ; iter++, k++)
      res[k]=a[iter->first]*b[iter->second]-a[iter->second]*b[iter->first] ;
}
//-----------------------------------
template <int d>
void Tools<d>::unitvec (vector <double> & v, int n)
{
  for (int i=0 ; i<d ; i++) v[i]=(i==n?1:0) ;
}
//-----------------------------------
template <int d>
void Tools<d>::orthonormalise (v1d & A) //Gram-Schmidt process
{
     static int first = 0 ; // cycle through the base vector as first vector (to be impartial, random would probably be better but hey ...

     // Let's get the base vectors first
     v2d base (d, v1d(d,0)) ;
     for (int i=0 ; i<d ; i++)
         for (int j=0 ; j<d ; j++)
            base[i][j] = A[j*d+(i+first)%d] ;

     for (int i=0 ; i<d ; i++)
     {
         for (int j=0 ; j<i ; j++)
             base[i] -= base[j] * (dot(base[i],base[j])) ;
         base[i] /= norm(base[i]) ;
     }

     for (int i=first ; i<first+d ; i++)
         for (int j=0 ; j<d ; j++)
             A[j*d+(i%d)] = base[i-first][j] ;

     //first++ ; if (first>=d) first=0 ;
}
//-----------------------------------
template <int d>
double Tools<d>::det (cv2d &M)
{
 double res=0 ;
 vector<vector<double>>submatrix ;
 submatrix.resize(d-1, vector<double>(d-1)) ;  
 for (int i=0 ; i<d ; i++)
 {
   for (int j=0 ; j<d ; j++)
     for (int k=0 ; k<d-1 ; k++)
     {
       if (j==i) continue ; 
       submatrix[k][j-(j>i?1:0)]=M[k][j] ; 
     }
   res += ((i+d-1)%2?-1:1) * M[d-1][i] * Tools<d-1>::det(submatrix) ;
 }
 return res ; 
}
template <> double Tools<2>::det(cv2d &M) { return M[0][0]*M[1][1]-M[1][0]*M[0][1] ; }
template <> double Tools<1>::det(cv2d &M) { return M[0][0] ; }
//----------------
template <int d>
double Tools<d>::det (cv1d &M)
{
 double res=0 ;
 vector<double> submatrix ;
 submatrix.resize((d-1)*(d-1)) ;  
 for (int i=0 ; i<d ; i++)
 {
   for (int j=0 ; j<d ; j++)
     for (int k=0 ; k<d-1 ; k++)
     {
       if (j==i) continue ; 
       submatrix[k*(d-1)+ j-(j>i?1:0)]=M[k*d+j] ; 
     }
   res += ((i+d-1)%2?-1:1) * M[(d-1)*d+i] * Tools<d-1>::det(submatrix) ;
 }
 return res ; 
}
template <> double Tools<2>::det(cv1d &M) { return M[0]*M[3]-M[1]*M[2] ; }
template <> double Tools<1>::det(cv1d &M) { return M[0] ; }
//==================================
template <int d>
vector<double> Tools<d>::inverse (cv1d &M)
{
  vector<double> res (d*d,0) ;
  vector<double> submatrix (d*d,0) ; 
  for (int i=0 ; i<d ; i++)
    for (int j=0 ; j<d ; j++)
    {
      submatrix = M ; 
      for (int dd=0 ; dd<d ; dd++)
        submatrix[i*d+dd]=(dd==j?1:0) ; 
      res[i*d+j]=Tools<d>::det(submatrix) ; 
    }
  double determinant = Tools<d>::det(M) ; 
  for (int i=0 ; i<d*d ; i++)
    res[i] /= determinant ; 
  Tools<d>::transpose_inplace(res) ; 
  return res ; 
}

//==================================
template <int d>
double Tools<d>::Volume (double R)
{
  if (d%2==0)
    return (pow(boost::math::double_constants::pi,d/2)*pow(R,d)/( boost::math::factorial<double>(d/2) )) ;
  else
  {
   int k=(d-1)/2 ;
   return(2* boost::math::factorial<double>(k) * pow(4*boost::math::double_constants::pi, k) *pow(R,d) / (boost::math::factorial<double>(d))) ;
  }
}
//----------------------------------------
template <int d>
double Tools<d>::InertiaMomentum (double R, double rho)
{
 if (d>MAXDEFDIM)
 {
  printf("[WARN] Inertia InertiaMomentum not guaranteed for dimension > %d\n", MAXDEFDIM) ;
 }

 double res ;
 if (d%2==0)
 {
   uint k=d/2 ;
   res=pow(boost::math::double_constants::pi,k)*pow(R, d+2) / boost::math::factorial<double>(k+1) ;
   return (res*rho) ;
 }
 else
 {
   uint k=(d-1)/2 ;
   res=pow(2,k+2) * pow(boost::math::double_constants::pi, k) * pow(R, d+2) / boost::math::double_factorial<double> (d+2) ;
   return (res*rho) ;
 }
}

//--------------------------------
template <int d>
double Tools<d>::hyperspherical_xtophi (cv1d &x, v1d &phi)
{
    double rsqr = normsq(x) ;
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
void Tools<d>::hyperspherical_phitox (double r, cv1d &phi, v1d &x)
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

/*
Analytical functions for the momentum of inertia (cf mat script)

(8*pi*R^5*rho)/15 d=3 double factorial of 5
(16*R^7*rho*pi^2)/105 d=5 double factorial of 7
(32*R^9*rho*pi^3)/945 d=7 double factorial of 9 etc
(64*R^11*rho*pi^4)/10395
(128*R^13*rho*pi^5)/135135
(256*R^15*rho*pi^6)/2027025
(512*R^17*rho*pi^7)/34459425
(1024*R^19*rho*pi^8)/654729075
(2048*R^21*rho*pi^9)/13749310575
(4096*R^23*rho*pi^10)/316234143225
(8192*R^25*rho*pi^11)/7905853580625
(16384*R^27*rho*pi^12)/213458046676875
(32768*R^29*rho*pi^13)/6190283353629375
(65536*R^31*rho*pi^14)/191898783962510625

(pi*R^4*rho)/2
(R^6*rho*pi^2)/6
(R^8*rho*pi^3)/24
(R^10*rho*pi^4)/120
(R^12*rho*pi^5)/720
(R^14*rho*pi^6)/5040
(R^16*rho*pi^7)/40320
(R^18*rho*pi^8)/362880
(R^20*rho*pi^9)/3628800
(R^22*rho*pi^10)/39916800
(R^24*rho*pi^11)/479001600
(R^26*rho*pi^12)/6227020800
(R^28*rho*pi^13)/87178291200
(R^30*rho*pi^14)/1307674368000 (factorial 15)
(R^32*rho*pi^15)/20922789888000
  */




//=====================================================================================================NETCDF
#ifdef NETCDF
int NetCDFWriter::initialise (string path, initializer_list< v2d > & list, vector <string> names)
{
int dimids[3] ;
int ret = nc_create((path+".nc").c_str(), NC_CLOBBER, &ncid) ;
if (ret) {printf("An error occured creating the netCDF file\n") ; return 0 ; }

nc_def_dim (ncid, "Grains", list.begin()->size(), dimensions) ;
nc_def_dim (ncid, "Scalar", 1, dimensions+1) ;
nc_def_dim (ncid, "Vector", Tools<d>::getdim(), dimensions+2) ;
nc_def_dim (ncid, "Tensor", Tools<d>::getdim()*Tools<d>::getdim(), dimensions+3) ;
nc_def_dim (ncid, "SkewTensor", Tools<d>::getdim()*(Tools<d>::getdim()-1)/2, dimensions+4) ;
nc_def_dim (ncid, "Time", NC_UNLIMITED, dimensions+5) ;

dimids[0]=dimensions[5] ; dimids[2]=dimensions[0] ; // Dimension order: Time, grains, dim
for (auto i=names.begin() ; i<names.end() ; i++)
{
 if (list.begin()->at(0).size() == 1) dimids[1]=dimensions[1] ;
 else if (list.begin()->at(0).size() == Tools<d>::getdim()) dimids[1]=dimensions[2] ;
 else if (list.begin()->at(0).size() == Tools<d>::getdim()*Tools<d>::getdim()) dimids[1]=dimensions[3] ;
 else if (list.begin()->at(0).size() == Tools<d>::getdim()*(Tools<d>::getdim()-1)/2) dimids[1]=dimensions[4] ;
 else {printf("ERR: Unknown dimension size to write in netCDF\n") ; return 1 ; }
 varid.push_back(0) ;
 nc_def_var(ncid, (*i).c_str() , NC_DOUBLE, 3, dimids, &(varid[varid.size()-1])) ;
}
nc_enddef(ncid) ;
first = false ;
return 0 ;
}
//--------------------------------------------------------------
int NetCDFWriter::saveNetCDF (initializer_list< v2d > & list)
{
size_t start[3], count[3] ;
for (auto i = varid.begin() ; i<varid.end() ; i++)
{
    auto & X=*(list.begin()) ;
    start[0]=timerecord ; start[1]=X.size() ;  ; start[2]=0 ;
    count[0]=count[1]=1 ; count[2]=X[0].size() ;
    for (auto j=X.begin() ; j < X.end() ; j++)
        nc_put_vara_double (ncid, *i , start, count, j->data());
}
timerecord++ ;
return timerecord ;
}
#endif



#ifdef NETCDF
#include <netcdf.h>
class NetCDFWriter {
public:
  int initialise (string path, initializer_list< v2d > & list, vector <string> names) ;
  int writeCDF (string path, initializer_list< v2d > & list, vector <string> & names) {if (first) {initialise(path, list, names); first=false;} saveNetCDF(list) ; return 0 ;  }
  ~NetCDFWriter() {if (!first) nc_close(ncid) ;}
private:
  int ncid ;
  int timerecord=0 ;
  int dimensions[6];
  vector <int> varid, stride ;
  bool first=true ;
  int saveNetCDF (initializer_list< v2d > & list) ;
} ;
#endif

#endif
/** @} */
