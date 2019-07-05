#include "IONDDEM.h"
#include <boost/math/special_functions/factorials.hpp>

struct Param {
  string dump ;
  int skipT=50 ;
  int maxT = 100 ;
  double rho=1.9098593171027443 ;
  vector <string> flags = {"RHO", "VAVG"} ;
  vector <int> boxes= {15,5,5} ;
  vector <vector <double> > boundaries ;
  vector <double> radius ;
  string save="" ;
} P;

void dispvector (v2d &u) {for (auto v:u) {for (auto w:v) printf("%g ", w) ; printf("\n") ; } fflush(stdout) ; }
void dispvector (v1d &u) {for (auto v:u) printf("%g ", v) ; printf("\n") ; fflush(stdout) ;}

double Volume (int d , double R) ; 
double InertiaMomentum (int d, double R, double rho) ;

//===================================================
int main (int argc, char * argv[])
{
 /*if (argc > 2)
 {
   P.dump = argv[2] ;
   P.selfset(argv[1]) ;
 }
 else*/
 P.dump=argv[1] ;

 P.maxT=100 ;

 //struct Data D ;
 // Extract path (last slash) for saving
 int res, t ;
 XMLReader XML(P.dump) ;
 int d=atoi(XML.tags.second["dimensions"].c_str()) ;
 XML.read_boundaries(P.boundaries) ;
 P.boundaries[1][0] = 10 ;
 dispvector(P.boundaries) ;
 XML.read_radius (P.radius) ;
 int N = P.radius.size() ;

 vector <double> mass, Imom ;

 for (int i=0 ; i<N ; i++)
 {
     mass.push_back(Volume(d,P.radius[i]) * P.rho) ;
     printf("%g ", mass[i]) ;
     Imom.push_back(InertiaMomentum(d,P.radius[i],P.rho)) ; 
 }

 Coarsing C(d, P.boxes, P.boundaries, P.maxT-P.skipT) ;
 C.setWindow("LibLucyND") ;
 C.set_flags(P.flags) ;
 C.grid_setfields() ;
 C.cT=-1 ;
 C.data.N=P.radius.size() ;
 C.data.mass = mass.data() ;
 C.data.Imom = Imom.data() ;

 vector <string> names ; vector<vector<vector<double>>> data ;
 for (int t=0 ; t<P.maxT ; t++)
 {
  res = XML.read_nextts(names, data) ;
  if (t<P.skipT) continue ;

  if (res!=0) break ;
  C.cT++ ;

  int delta=find(names.begin(), names.end(), "Position")-names.begin() ;
  if (C.data.pos.size()==0) {C.data.pos.resize(d) ; for (auto & v:C.data.pos) {v=(double*) (malloc (sizeof(double)*C.data.N)) ; }}
  //printf("\n") ; fflush(stdout) ;
  for (int j=0 ; j<C.data.N ; j++) for (int k=0 ;k<d ; k++) {C.data.pos[k][j] = data[delta][j][k] ; /*printf("%g ",C.data.pos[k][j]) ; */}

  delta=find(names.begin(), names.end(), "Velocity")-names.begin() ;
  if (delta!=(names.end()-names.begin()))
  {
    if (C.data.vel.size()==0) {C.data.vel.resize(d) ; for (auto & v:C.data.vel) {v=(double*) (malloc (sizeof(double)*C.data.N)) ; } }
    for (int j=0 ; j<C.data.N ; j++) for (int k=0 ;k<d ; k++) C.data.vel[k][j] = data[delta][j][k] ;
  }

  C.pass_1() ;
  //C.compute_fluc_vel() ;
  //C.compute_fluc_rot() ;
  //C.pass_2() ;
  //C.pass_3() ;

 printf("%d \n", t) ;
 }

 //C.mean_time() ;
 C.write_vtk("Coarsed") ;
 C.write_NrrdIO("Coarsed") ;





/*vector <double *> pos, vel, omega ; //Vectors

v2d vel_fluc, rot_fluc ; ///< Should not be externally provided but calculated, using the function *Coarsing::compute_fluc_vel()*

int Ncf ;
double * id1, *id2 ;
vector <double *> pospq, lpq, fpq, mpq, mqp ;
 D*/





}
//==============================================
double Volume (int d, double R)
{
  if (d%2==0)
    return (pow(boost::math::double_constants::pi,d/2)*pow(R,d)/( boost::math::factorial<double>(d/2) )) ;
  else
  {
   int k=(d-1)/2 ;
   return(2* boost::math::factorial<double>(k) * pow(4*boost::math::double_constants::pi, k) *pow(R,d) / (boost::math::factorial<double>(d))) ;
  }
}
#define MAXDEFDIM 30
//----------------------------------------
double InertiaMomentum (int d , double R, double rho)
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
