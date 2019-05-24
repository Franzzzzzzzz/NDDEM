#include "../Dem/Tools.h"
#include "io.h"

void phi2color (vector<uint8_t>::iterator px, cv1d & phi, int d) ;
int viewpermute (v1d & View, int d) ;
void spaceloop (vector<string> & FileList, v1d View, int nrotate, int dim, int time, cv2d & X, cv1d & R, cv2d & A) ;
void timeloop (vector<string> & FileList, v1d View, int nrotate, const vector <int> & timelst, uint timeidx, cv3d & X, cv1d & R, cv3d & A) ;
void Render (vector <string> & filerendered, cv1d & View, int nrotate, int time, cv2d &X, cv1d & R, cv2d &A) ;

class Rendering {
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

  // function
  int set_grid (int nb) ; 

} ;
