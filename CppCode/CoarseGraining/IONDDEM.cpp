#include "IONDDEM.h"


struct Param {
  string dump ;
  int skipT=0 ; 
  int maxT = 100 ; 
  double rho=1 ; 
  vector <string> flags = {"RHO"} ; 
  vector <int> boxes= {5,5,5} ;
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

 struct Data D ; 
 // Extract path (last slash) for saving 
 
 int res, t ;
 XMLReader XML(P.dump) ;
 int d=atoi(XML.tags.second["dimensions"].c_str()) ; 
 XML.read_boundaries(P.boundaries) ; 
 P.boundaries[1][0] = 5 ;
 dispvector(P.boundaries) ;
 XML.read_radius (P.radius) ;
 D.N = P.radius.size() ; 
 
 vector <double> mass, Imom ;
 
 for (int i=0 ; i<D.N ; i++) 
 {
     mass.push_back(4/3. * M_PI * P.radius[i] * P.radius[i] * P.radius[i] * P.rho) ; 
     Imom.push_back( 2/5. * mass[i] * P.radius[i] * P.radius[i]) ; 
 }
 D.mass = mass.data() ;
 D.Imom = Imom.data() ;
 
 printf("A"); fflush(stdout) ; 
 
 Coarsing C(d, P.boxes, P.boundaries, P.maxT) ; 
 C.set_flags(P.flags) ; 
 C.grid_setfields() ;
 C.cT=-1 ; 
 printf("B"); fflush(stdout) ; 
 
 vector <string> names ; vector<vector<vector<double>>> data ;
 for (int t=0 ; t<P.maxT ; t++)
 {
  res = XML.read_nextts(names, data) ;   
  if (res!=0) break ; 
  C.cT++ ; 
  
  int delta=find(names.begin(), names.end(), "Position")-names.begin() ; 
  if (D.pos.size()==0) {D.pos.resize(d) ; for (auto & v:D.pos) {v=(double*) (malloc (sizeof(double)*D.N)) ; }}
  for (int j=0 ; j<D.N ; j++) for (int k=0 ;k<d ; k++) D.pos[k][j] = data[delta][j][k] ;
  
  /*delta=find(names.begin(), names.end(), "Velocity")-names.begin() ; 
  if (delta!=(names.end()-names.begin()))
  {
    if (D.vel.size()==0) {D.vel.resize(d) ; for (auto & v:D.vel) {v=(double*) (malloc (sizeof(double)*D.N)) ; } }
    for (int j=0 ; j<D.N ; j++) for (int k=0 ;k<d ; k++) D.vel[k][j] = data[delta][j][k] ;
  }*/
  
 printf("D"); fflush(stdout) ; 
  C.pass_1() ; 
  //C.compute_fluc_vel() ; 
  //C.compute_fluc_rot() ;
  //C.pass_2() ;
  //C.pass_3() ;
  
 printf("%d ", t) ; 
 }
 
 printf("%d ", t) ; 
 
 
 
 
 
/*vector <double *> pos, vel, omega ; //Vectors

v2d vel_fluc, rot_fluc ; ///< Should not be externally provided but calculated, using the function *Coarsing::compute_fluc_vel()*

int Ncf ; 
double * id1, *id2 ;
vector <double *> pospq, lpq, fpq, mpq, mqp ; 
 D*/
 
 
 
 
    
}




