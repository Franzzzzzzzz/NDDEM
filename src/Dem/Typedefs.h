/** \addtogroup DEM Discrete Element Simulations
 * This module handles the Discrete Element Simulations.
 *  @{ */

#ifndef TYPEDEFS
#define TYPEDEFS
using namespace std ;
using uint = unsigned int ;
using v1d = vector < double > ;
using v2d = vector < vector <double> > ;
using v3d = vector<vector<vector<double>>>;
using v1f = vector <float> ;
using cv1d= const vector <double> ;
using cv2d= const vector < vector <double> > ;
using cv3d = const vector<vector<vector<double>>> ;
using cv1f = const vector <float> ;
using bitdim = unsigned int ;
using v1i = vector <int> ;
enum DataValue {radius, mass, Imom, pos, vel, omega, id1, id2, pospq, lpq, fpq, mpq, mqp, extra_named} ;

enum ContactModels {HOOKE=0, HERTZ} ; 
enum class WallType {PBC=0, WALL=1, MOVINGWALL=2, SPHERE=3, ROTATINGSPHERE=4, PBC_LE=5, ELLIPSE=6} ; ///< Wall types

#endif

/** @} */
