#include "IONDDEM.h"


struct Param {
  string dump ;
  int skipT=0 ; 
  int maxT = 100 ; 
  vector <string> flags = {"RHO", "VAVG", "TC", "TK",} ; 
  int dim=4 ; //TODO
  vector <int> boxes= {10,10,10,10} ;
  vector <double> boundaries; 
  vector <double> radius ; 
  string save="" ; 
} P; 




int main (int argc, char * argv[])
{
 P.dump = argv[1] ; 

 struct Data D ; 
 // Extract path (last slash) for saving 
 
 XMLReader XML(P.dump) ;
 XML.get_boundaries(P.boundaries) ; 
 XML.get_radius (P.radius) ;
 
 D.N = P.radius.size() ; 
 
 vector <double> mass, Imom ;
 
 for (int i=0 ; i<D.N ; i++) 
 {
     mass[i] = 4/3. * M_PI * P.radius[i] * P.radius[i] * P.radius[i] * P.rho ; 
     Imom[i] = 2/5. * mass[i] * P.radius[i] * P.radius[i] ; 
 }
 D.mass = mass.data() ;
 D.Imom = Imom.data() ; 
 
 vector <string> names ; vector<vector<vector<double>>> & data ;
 for (int t=0 ; t<10 ; t++)
 {
  XML.read_nextts(names, data) ;   
     
  int delta=find(names.begin(), names.end(), "Position")-names.begin() ; 
  D.
 }
 
 
vector <double *> pos, vel, omega ; //Vectors

v2d vel_fluc, rot_fluc ; ///< Should not be externally provided but calculated, using the function *Coarsing::compute_fluc_vel()*

int Ncf ; 
double * id1, *id2 ;
vector <double *> pospq, lpq, fpq, mpq, mqp ; 
 
 D
 
 
 
 
    
}




