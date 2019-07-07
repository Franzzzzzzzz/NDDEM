#include <cstdlib>
#include <cstdio>
#include <vector>
#include "Typedefs.h"
#include <boost/random.hpp>

#ifdef NETCDF
#include <netcdf.h>
#endif

#ifdef NRRDIO
#include "NrrdIO-1.11.0-src/NrrdIO.h"
#endif

using namespace std ;

//=========================================================
/// Coarse graining point
class CGPoint
{
public :
    CGPoint(int dd, v1d loc): natom(0), phi(0) {d=dd ; location=loc ; }

    v2d fields ;    ///< 1st dimension is time, second are fields
    v1d location ;  ///< Location of the coarse graining point
    //Useful things
    vector <int> neighbors ; ///< All the neighbors of the point given the window. 1st index is the point itself
    double natom ;
    double phi ;

private :
    int d ;
} ;
//-------------------------
/// Contains Field informations
struct Field {
 int flag ;
 string name ;
 string type ;
};
//-------------------------
/// Data structure handling point data and contact data
struct Data {
public:
    Data () : mass(NULL), Imom(NULL), id1(NULL), id2(NULL) {}
int N ;
double * mass, *Imom ; // Scalars
vector <double *> pos, vel, omega ; //Vectors

v2d vel_fluc, rot_fluc ; ///< Should not be externally provided but calculated, using the function *Coarsing::compute_fluc_vel()*

int Ncf ;
double * id1, *id2 ;
vector <double *> pospq, lpq, fpq, mpq, mqp ;

// Some useful functions
int Nnonper=-1 ; 
int random_test (int N, int Ncf, int d, v2d box ) ; ///< Randomly fill the data structure
int compute_lpq (int d) ; ///< Compute lpq from contact id's and atom locations
int periodic_atoms (int d, v2d bounds, int pbc, v1d Delta, bool omegainclude) ; 
int clean_periodic_atoms () {if (Nnonper==-1) printf("ERR: must call periodic_atoms before cleaning the periodic_atoms\n") ; else N=Nnonper ; return 0 ; }
} ;

//------------------------------------------------------
#include "WindowLibrary.h"

//=========================================================
/// Main Coarse graining class
class Coarsing
{
public :
    Coarsing (int dd, v1i nnpt, v2d bbox, int T) : flags(0), cT(0)
    {
        d=dd ; npt=nnpt ; box=bbox ; Time=T ;
        dx.resize(d, 0) ;
        for (int i=0 ; i<d ; i++)
          dx[i]=((box[1][i]-box[0][i])/double(npt[i])) ;

        double w= (*std::min_element(dx.begin(),dx.end())*2) ; // w automatically set
        cutoff=2.5*w ; //TODO
        printf("Window and cutoff: %g %g \n", w, cutoff) ;
        //for (int i=0 ; i<d ; i++)
        // printf("%d %d %g %g %g|", d, npt[i], box[1][i], box[0][i], dx[i]) ; fflush(stdout) ;
        grid_generate() ;
        grid_neighbour() ;
        set_field_struct() ;
        Window = new LibLucy3D( &data, w, d) ;
    }

    int d ; ///< Number of dimensions
    int Npt; ///<Number of coarse graining points
    int Time; ///<Total timesteps
    int cT ; ///< Current timestep
    double cutoff ; ///< CG width, and cutoff
    vector <class CGPoint> CGP ; ///< List of Coarse Graining points
    vector <int> npt; ///< Number of points per dimension
    vector <int> nptcum ; ///< Cumulated number of points per dimensions (usefull for quick finding of the closest CG for a grain)
    v1d dx ;
    v2d box ;
    LibBase * Window ;

    // Fields variable and function
    unsigned int flags ; ///< Flags deciding which fields to coarse-grain
    vector <string> Fields, Fname ; ///< Flagged field names
    vector <int> Fidx, Ftype ; ///< Where the fields is referenced in the fields vector in the CGPoint. -1 if not flagged
    vector <struct Field > FIELDS ; ///< All allowed fields (initialized in grid_getfields)
    int get_id(string nm) ;
    struct Field * get_field(string nm) ;
    int set_flags (vector <string> s) ;

    // Grid functions
    int set_field_struct() ; //< Set the FIELDS structure, with all the different CG properties that can be computed.
    int setWindow (string windowname) ;
    int grid_generate() ;
    int grid_neighbour() ;
    int grid_setfields() ;
    v1d grid_getfields() ;
    v2d get_bounds() ;
    CGPoint * reverseloop (string type) ; //< go through the table in reverse order of the dimensions (for the writing phase essentially)
    int find_closest (int id) ;
    int find_closest_pq (int id) ;
    v1d interpolate_vel(int id) { return interpolate_vel_nearest (id) ; }
    v1d interpolate_rot(int id) { return interpolate_rot_nearest (id) ; }
    v1d interpolate_vel_nearest (int id) ;
    v1d interpolate_rot_nearest (int id) ;

    int idx_FastFirst2SlowFirst (int n) ;

    // Windowing functions
    //double window(double r) {Lucy(r) ; }
    //double window_int (v1d r1, v1d lpq, v1d x) {printf("Numerical integration of wpqf not implemented\n") ; } ///< Numerical integration: not implemented
    //double window_int(double r1, double r2) {return window_avg(r1, r2) ; } ///< Overload to avoid integration ...
    //double window_avg (double r1, double r2) {return (0.5*(Lucy(r1)+Lucy(r2))) ; }
    //double Lucy (double r) {static double cst=105./(16*M_PI*w*w*w) ; if (r>=w) return 0 ; else {double f=r/w ; return (cst*(-3*f*f*f*f + 8*f*f*f - 6*f*f +1)) ; }}
    double normdiff (v1d a, v1d b) {double res=0 ; for (int i=0 ; i<d ; i++) res+=(a[i]-b[i])*(a[i]-b[i]) ; return (sqrt(res)) ; } ;
    // Coarse graining functions
    int pass_1 () ; ///< Coarse-grain anything based on particles (not contacts) which does not need fluctuating quantities
    int pass_2 () ; ///< Coarse-grain anything based on particles (not contacts) which needs fluctuating quantities (call the compute_fluc_ functions before)
    int pass_3 () ; ///< Coarse-grain anything based on contact informations.

    // Data handling functions
    int compute_fluc_vel () ;
    int compute_fluc_rot () ;
    struct Data data ;

    // Time and output handling
    int mean_time() ;
    int write_vtk(string sout) ;
    int write_netCDF (string sout) ;
    int write_NrrdIO (string path) ;
} ;
