#ifndef TOOLS
#define TOOLS
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <vector>
#include <cmath>
#include <initializer_list>
#include "Typedefs.h"

#include <boost/math/special_functions/factorials.hpp>
#include <boost/random.hpp>

#define MAXDEFDIM 30


v1d operator* (v1d a, double b) ;
v1d operator* (v1d a, cv1d b)   ; 
v1d operator+ (v1d a, double b) ; 
v1d operator+ (v1d a, cv1d b)   ; 
v1d operator- (v1d a, double b) ; 
v1d operator- (v1d a, cv1d b)   ; 
v1d operator- (v1d a)           ; 
v1d operator/ (v1d a, double b) ; 
v1d & operator-= (v1d & a, cv1d b) ; 
v1d & operator*= (v1d & a, double b);
v1d & operator+= (v1d & a, cv1d b) ; 
v1d & operator/= (v1d & a, double b) ; 

class Tools 
{
public:
static void initialise (int dd) ; 
static bool check_initialised (int dd) {return (dd==d) ; }

static int savetxt(char path[], const v2d & table, char header[]) ;
static void unitvec (vector <double> & v, int d, int n) ; 
static v1d unitvec (int n) {v1d res (d,0) ; res[n]=1 ; return res ; }
static double norm (const vector <double> & a) {double res=0 ; for (int i=0 ; i<d ; i++) res+=a[i]*a[i] ; return (sqrt(res)) ; }
static double normdiff (cv1d & a, cv1d & b) {double res=0 ; for (int i=0 ; i<d ; i++) res+=(a[i]-b[i])*(a[i]-b[i]) ; return (sqrt(res)) ; }
static double normsq (const vector <double> & a) {double res=0 ; for (int i=0 ; i<d ; i++) res+=a[i]*a[i] ; return (res) ; }
static double normdiffsq (cv1d & a, cv1d & b) {double res=0 ; for (int i=0 ; i<d ; i++) res+=(a[i]-b[i])*(a[i]-b[i]) ; return (res) ; }
static double skewnorm (cv1d & a) {double res=0 ; for (int i=0 ; i<d*(d-1)/2 ; i++) res+=a[i]*a[i] ; return (sqrt(res)) ; }
static double skewnormsq (cv1d & a) {double res=0 ; for (int i=0 ; i<d*(d-1)/2 ; i++) res+=a[i]*a[i] ; return (res) ; }
static double dot (cv1d & a, cv1d & b) {double res=0; for (int i=0 ; i<d ; i++) res+=a[i]*b[i] ; return (res) ; } 
static v1d dbl2vec (double v) {v1d res (1,v) ; return res ; }
static void setzero(v2d & a) {for (int i=0 ; i<a.size() ; i++) for (int j=0 ; j<a[0].size() ; j++) a[i][j]=0 ; }
static void setzero(v1d & a) {for (int i=0 ; i<a.size() ; i++) a[i]=0 ; }
static void setgravity(v2d & a, v1d &g, v1d &m) {for (int i=0 ; i<a.size() ; i++) a[i]=g*m[i] ; }
static void savecsv (char path[], cv2d & X, cv1d &r) ; 
static void savevtk (char path[], cv2d & X) ; 

static int write1D (char path[], v1d table) ; 
static int writeinline(initializer_list< v1d >) ; 
static int writeinline_close(void) ; 

static v1d randomize_vec (cv1d v) ;

static double Volume (double R) ; 
static double InertiaMomentum (double R, double rho) ; 

static v1d  skewmatvecmult (cv1d & M, cv1d &v) ;
static void skewmatvecmult (v1d & r, cv1d & M, cv1d &v) ;
static v1d  skewmatsquare  (cv1d &A) ;
static void skewmatsquare  (v1d &r, cv1d &A) ;
static v1d  skewexpand     (cv1d &A) ;
static void skewexpand     (v1d & r, cv1d &A) ;
static v1d  matmult (cv1d &A, cv1d &B) ;
static void matmult (v1d &r, cv1d &A, cv1d &B) ;
static v1d  wedgeproduct (cv1d &a, cv1d &b) ; 
static void wedgeproduct (v1d &res, cv1d &a, cv1d &b) ; // Overloaded faster operation

// Faster vector operations
static void vMul  (v1d & res, cv1d &a, double b)  {for (int i=0 ; i<a.size() ; i++) res[i]=a[i]*b ; }
static void vMul  (v1d & res, cv1d &a, cv1d & b)  {for (int i=0 ; i<a.size() ; i++) res[i]=a[i]*b[i] ; }
static void vMinus (v1d & res, cv1d &a, cv1d & b) {for (int i=0 ; i<a.size() ; i++) res[i]=a[i]-b[i] ; }
static void vPlus (v1d & res, cv1d &a, double b)  {for (int i=0 ; i<a.size() ; i++) res[i]=a[i]+b ; }
static void vPlus (v1d & res, cv1d &a, cv1d & b)  {for (int i=0 ; i<a.size() ; i++) res[i]=a[i]+b[i] ; }
static void vDiv   (v1d & res, cv1d &a, double b) {for (int i=0 ; i<a.size() ; i++) res[i]=a[i]/b ; }
static void vDiv   (v1d & res, cv1d &a, cv1d & b) {for (int i=0 ; i<a.size() ; i++) res[i]=a[i]/b[i] ; }
static void vAdd (v1d &res , cv1d &a, cv1d &b)    {for (int i=0 ; i<a.size() ; i++) res[i] += a[i]+b[i] ; }
static void vAddScaled (v1d &res , double d, cv1d &a, cv1d &b) {for (int i=0 ; i<a.size() ; i++) res[i] += d*(a[i]+b[i]) ; }
static void vAddScaled (v1d &res , double d, cv1d &a)          {for (int i=0 ; i<a.size() ; i++) res[i] += d*a[i] ; }
static void vSub (v1d &res , cv1d &a, cv1d &b)                 {for (int i=0 ; i<a.size() ; i++) res[i] -= (a[i]+b[i]) ; }
static void vSubScaled (v1d &res , double d, cv1d &a)          {for (int i=0 ; i<a.size() ; i++) res[i] -= d*a[i] ; }
static void vScaledSum (v1d &res , double d, cv1d &a, cv1d &b) {for (int i=0 ; i<a.size() ; i++) res[i] += d*a[i]+b[i] ; }


static v1d Eye ; 
static boost::random::mt19937 rng ; 
static boost::random::uniform_01<boost::mt19937> rand ;  

static const int getdim (void) {return d;} // Essentially so that the NetCDF class has access to d ...


private:
static int d ;   
static vector < vector <int> > MSigns ; 
static vector < vector <int> > MIndexAS ; 
static vector < pair <int,int> > MASIndex ;
static vector <FILE *> outs ; 
} ; 




#ifdef NETCDF
#include <netcdf.h>
class NetCDFWriter {
public:
  int initialise (string path, initializer_list< v2d > & list, vector <string> names) ;
  int writeCDF (string path, initializer_list< v2d > & list, vector <string> & names) {if (first) {initialise(path, list, names); first=false;} saveNetCDF(list) ; }
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
