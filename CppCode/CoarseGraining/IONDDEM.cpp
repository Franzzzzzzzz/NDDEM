#include "IONDDEM.h"
#include <boost/math/special_functions/factorials.hpp>

struct Param {
  string dump ;
  int skipT ;
  int maxT ;
  double rho ;
  vector <string> flags ;
  vector <int> boxes ;
  vector <vector <double> > boundaries ;
  int pbc ;
  vector<double> Delta ;
  vector <double> radius ;
  string save;
} ;

// Struct init
struct Param P2 {
  "/Users/FGuillard/Dropbox/DEM_ND/Samples/D2/dump.xml",                                 // dump
  50,                                 // skipT
  450,                                // maxT
  1.2732395447351628,                 // rho
  {"RHO", "VAVG"},                    // flags
  {25,1},                             // boxes #
  {{}},                               // Boundaries
  0b10,                               // PBC (fisrt dimension is LSD)
  {20, 5},                            // Deltas (used for pbcs)
  {},                                 // radii
  "CoarseD2"                          // save location
};

struct Param P3 {
  "/Users/FGuillard/Dropbox/DEM_ND/Samples/D3/dump.xml",                                 // dump
  50,                                 // skipT
  450,                                // maxT
  1.9098593171027443,                 // rho
  {"RHO", "VAVG"},                    // flags
  {25,1,1},                           // boxes #
  {{}},                               // Boundaries
  0b110,                              // PBC (fisrt dimension is LSD)
  {20, 5, 3.4},                       // Deltas (used for pbcs)
  {},                                 // radii
  "CoarseD3"                          // save location
};

struct Param P4 {
  "/Users/FGuillard/Dropbox/DEM_ND/Samples/D4/dump.xml",                                 // dump
  50,                                 // skipT
  450,                                // maxT
  3.242277876554809,                  // rho
  {"RHO", "VAVG"},                    // flags
  {25,1,1,1},                           // boxes #
  {{}},                               // Boundaries
  0b1110,                              // PBC (fisrt dimension is LSD)
  {20, 5, 3.4, 3.4},                       // Deltas (used for pbcs)
  {},                                 // radii
  "CoarseD4"                          // save location
};

struct Param P5 {
  "/Users/FGuillard/Dropbox/DEM_ND/Samples/D5/dump.xml",                                 // dump
  50,                                 // skipT
  450,                                // maxT
  6.079271018540266,                  // rho
  {"RHO", "VAVG"},                    // flags
  {25,1,1,1,1},                           // boxes #
  {{}},                               // Boundaries
  0b11110,                              // PBC (fisrt dimension is LSD)
  {20, 5, 3.4, 3.4, 3.4},                       // Deltas (used for pbcs)
  {},                                 // radii
  "CoarseD5"                          // save location
};

struct Param P6 {
  "/Users/FGuillard/Dropbox/DEM_ND/Samples/D6/dump.xml",                                 // dump
  50,                                 // skipT
  230,                                // maxT
  12.384589222348605,                  // rho
  {"RHO", "VAVG"},                    // flags
  {25,1,1,1,1,1},                           // boxes #
  {{}},                               // Boundaries
  0b111110,                              // PBC (fisrt dimension is LSD)
  {20, 5, 3.4, 3.4, 3.4, 3.4},                       // Deltas (used for pbcs)
  {},                                 // radii
  "CoarseD6"                          // save location
};

struct Param PTest {
  "/Users/FGuillard/Dropbox/DEM_ND/Samples/D4/dump.xml",                                 // dump
  50,                                 // skipT
  450,                                // maxT
  3.242277876554809,                  // rho
  {"RHO", "VAVG"},                    // flags
  {25,3,4,5},                           // boxes #
  {{}},                               // Boundaries
  0b1110,                              // PBC (fisrt dimension is LSD)
  {20, 5, 3.4, 3.4},                       // Deltas (used for pbcs)
  {},                                 // radii
  "CoarseD4Test"                          // save location
};

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

 auto P=P2 ;

 //P.dump=argv[1] ;

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
     Imom.push_back(InertiaMomentum(d,P.radius[i],P.rho)) ;
 }

 Coarsing C(d, P.boxes, P.boundaries, P.maxT-P.skipT) ;
 C.setWindow("LibLucyND", 1.5) ;
 C.set_flags(P.flags) ;
 C.grid_setfields() ;
 auto Bounds = C.get_bounds() ;
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

  C.data.periodic_atoms (d, Bounds, P.pbc, P.Delta, false) ;
  C.pass_1() ;
  C.data.clean_periodic_atoms() ;
  //C.compute_fluc_vel() ;
  //C.compute_fluc_rot() ;
  //C.pass_2() ;
  //C.pass_3() ;

 printf("%d \n", t) ; fflush(stdout) ;
 }

 C.mean_time() ;
 //C.write_vtk("Coarsed") ;
 C.write_NrrdIO(P.save.c_str()) ;

printf("\nA deallocation error may appear at the end. I am not quite sure where that come from (apparently the realloc in periodic_atoms leaves some stuff behind). Hopefully should not affect anything since it is the final deallocaiton as the program exits. \n") ; fflush(stdout) ;




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
   unsigned int k=d/2 ;
   res=pow(boost::math::double_constants::pi,k)*pow(R, d+2) / boost::math::factorial<double>(k+1) ;
   return (res*rho) ;
 }
 else
 {
   unsigned int k=(d-1)/2 ;
   res=pow(2,k+2) * pow(boost::math::double_constants::pi, k) * pow(R, d+2) / boost::math::double_factorial<double> (d+2) ;
   return (res*rho) ;
 }
}
