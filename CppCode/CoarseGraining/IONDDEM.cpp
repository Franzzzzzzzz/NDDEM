#include "IONDDEM.h"


struct Param {
  string dump ;
  int skipT=50 ;
  int maxT = 100 ;
  double rho=1 ;
  vector <string> flags = {"RHO", "VAVG"} ;
  vector <int> boxes= {4,3,8} ;
  vector <vector <double> > boundaries ;
  vector <double> radius ;
  string save="" ;
} P;

void dispvector (v2d &u) {for (auto v:u) {for (auto w:v) printf("%g ", w) ; printf("\n") ; } fflush(stdout) ; }
void dispvector (v1d &u) {for (auto v:u) printf("%g ", v) ; printf("\n") ; fflush(stdout) ;}

//===================================================
int main (int argc, char * argv[])
{
 P.dump = argv[1] ;

 P.maxT=100 ;

 //struct Data D ;
 // Extract path (last slash) for saving
 printf("A") ; fflush(stdout);
 int res, t ;
 XMLReader XML(P.dump) ;
 int d=atoi(XML.tags.second["dimensions"].c_str()) ;
 XML.read_boundaries(P.boundaries) ;
 P.boundaries[1][0] = 20 ;
 dispvector(P.boundaries) ;
 XML.read_radius (P.radius) ;
 int N = P.radius.size() ;

 vector <double> mass, Imom ;

 for (int i=0 ; i<N ; i++)
 {
     mass.push_back(4/3. * M_PI * P.radius[i] * P.radius[i] * P.radius[i] * P.rho) ;
     Imom.push_back(2/5. * mass[i] * P.radius[i] * P.radius[i]) ;
 }

 Coarsing C(d, P.boxes, P.boundaries, P.maxT-P.skipT) ;
 C.setWindow("LibRectND") ; 
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

 C.mean_time() ;
 C.write_vtk("Coarsed") ;
 C.write_NrrdIO("Coarsed") ;





/*vector <double *> pos, vel, omega ; //Vectors

v2d vel_fluc, rot_fluc ; ///< Should not be externally provided but calculated, using the function *Coarsing::compute_fluc_vel()*

int Ncf ;
double * id1, *id2 ;
vector <double *> pospq, lpq, fpq, mpq, mqp ;
 D*/





}
