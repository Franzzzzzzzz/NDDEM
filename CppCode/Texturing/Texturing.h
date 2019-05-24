#include "../Dem/Tools.h"
#include "io.h"

#include <thread>

void phi2color (vector<uint8_t>::iterator px, cv1d & phi, int d, vector<vector<float>> & colors) ;

class Timestep {
public: 
    v2d X, A ; 
} ; 


class Texturing {
public :
  int Nlambda=32, Ntheta=32 ;
  vector<vector<float>> colors = {
      {1,1,0},
      {0,1,1},
      {1,0,1},
      {1,1,0},
      {0,1,1},
      {1,0,1}} ;

  vector<vector<vector<float>>> allcolors = {
      {{231./256., 37./256., 100./256.}}, // official NDDEM pink
      {{1,1,0},{0,1,1}}};

  v1d lambdagrid, thetagrid ;
  string DirectorySave ;
  uint d ; int N ;
  v2d Boundaries ;
  
  vector <Timestep> Ts ;
  vector <int> TsName ; 
  v1d R ; 
  v1d View ; 
  vector<int> ViewPoint ; 
  
  bool justloaded ; 
  vector<vector<string>> FileList ; 
  vector <std::thread> Threads;

  // function
  int initialise (map <string,string> & args) ; 
  int clean() ;
  int set_grid (int nb) ; 
  void spaceloop (v1d View, uint tsint, int nrotate, int dim) ;
  void timeloop  (v1d View, uint tsint, int nrotate) ; 
  int MasterRender(map<string,string>) ; 
  void Render (vector <string> & filerendered, cv1d & View, int nrotate, int time, cv2d &X, cv1d & R, cv2d &A) ;
  int viewpermute (v1d & View, int d) ;
  void rescale (v1f & c, cv1f sum) ; 
  void filepathname (char * path, int n, int time, cv1d & View);
} ;
