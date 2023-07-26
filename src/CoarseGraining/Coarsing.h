/** \addtogroup CoarseGraining Coarse-graining
 *
 *  This modules handles the coarse graining in any number of dimensions, and has some neat features that are not that common even for 3D coarse graining.
 *
 *  In particular, the following fields can be requested, equation numbering is from Babic, Marijan. "Average balance equations for granular materials." International journal of engineering science 35.5 (1997): 523-548.:<br>
 * "RHO"   "SCALAR"    Eq 36 Density<br>
 *  "I"     "SCALAR"    Eq 65 Moment of Inertia<br>
 *  "VAVG"  "VECTOR"    Eq 38 Average Velocity<br>
 *  "TC"    "TENSOR"    Eq 63 Contact stress<br>
 *  "TK"    "TENSOR"    Eq 62 Kinetic stress<br>
 *  "ROT"   "VECTOR"    Eq 64 Internal spin density<br>
 *  "MC"    "TENSOR"    Eq 67 Contact couple stress tensor<br>
 *  "MK"    "TENSOR"    Eq 66 Kinetic couple stress tensor<br>
 *  "mC"    "VECTOR"    Eq 68 spin supply from contacts<br>
 *  "EKT"   "SCALAR"     VAVG^2/2 Kinetic energy density of average velocity<br>
 *  "eKT"   "SCALAR"    Eq 69 Kinetic energy density of fluctuating motion<br>
 *  "EKR"   "SCALAR"     (W.K.W)/2, kin energy of avg rotational motion, W angular velocity vector, K moment of inertia tensor<br>
 *  "eKR"   "SCALAR"    Eq 70 Kin energy of fluctuating rotational motion<br>
 *  "qTC"   "VECTOR"    Eq 72<br>
 *  "qTK"   "VECTOR"    Eq 71<br>
 *  "qRC"   "VECTOR"    Eq 74<br>
 *  "qRK"   "VECTOR"    Eq 73<br>
 *  "zT"    "SCALAR"    Eq 75<br>
 *  "zR"    "SCALAR"    Eq 76<br>
 *
 *  @{ */

#ifndef COARSING_H
#define COARSING_H

#include <cstdlib>
#include <cstdio>
#include <vector>
#include "Typedefs.h"
#include <boost/random.hpp>
#include <fstream>
#include <boost/math/special_functions/factorials.hpp>
#include <boost/math/special_functions/beta.hpp>
#include <boost/crc.hpp>
#include <map>

#ifdef NETCDF
#include <netcdf.h>
#endif

#ifdef NRRDIO
#include "../NrrdIO-1.11.0-src/NrrdIO.h"
#endif

#ifdef MATLAB
#include "mat.h"
#endif

using namespace std ;

double Volume (int d , double R) ; ///< Compute a sphere volume in dimension D

enum TensorOrder {NONE=-1, SCALAR=0, VECTOR=1, TENSOR=2} ;
enum FieldType {Defined, Particle, Fluctuation, Contact} ;
enum AverageType {None, Final, Intermediate, Both} ;
enum Pass {Pass1=1, Pass2=2, Pass3=4, Pass4=8, Pass5=16,
           VelFluct=256, RotFluct=512} ;

inline Pass operator|(Pass a, Pass b){return static_cast<Pass>(static_cast<int>(a) | static_cast<int>(b));}
//inline Pass operator|=(Pass a, const Pass b){return static_cast<Pass>(static_cast<int>(a) | static_cast<int>(b));} // Not working for some reason
inline bool operator& (Pass a, Pass b) { return (static_cast<int>(a) & static_cast<int>(b)) ; }

//=========================================================
/** \brief Data computed for a single coarse graining point */
class CGPoint
{
public :
    CGPoint(int dd, v1d loc) {d=dd ; location=loc ; }

    v2d fields ;    ///< 1st dimension is time, second are fields
    v1d location ;  ///< Location of the coarse graining point
    //Useful things
    vector <int> neighbors ; ///< All the neighbors of the point given the window. 1st index is the point itself
    int d ; ///< Dimension
} ;
//-------------------------
/// Contains Field informations
struct Field {
 uint64_t flag ; ///< Flag for the given field
 string name ; ///< Name for the given field
 TensorOrder type ; ///< Tensorial order of the field: SCALAR, VECTOR or TENSOR
 FieldType ftype ; ///< Mainly used to identified the type of user defined fields
 Pass passlevel ; ///< Identify at which moment the field gets calculated
 int datalocation = -1; ///< For extra fields, identify the location in the vector of extra data
};
//-------------------------
/// Data structure handling point data and contact data
struct Data {
public:
    Data () : radius(nullptr), mass(nullptr), Imom(nullptr), id1(nullptr), id2(nullptr) {}
int N = 0 ; ///< Number of particles
double * radius ; ///<Particle radius
double * mass ; ///< Particle masses
double *Imom ; ///< Particle moment of inertia
vector <double *> pos ; ///< Particle positions
vector <double *> vel ;  ///<Particle velocity
vector <double *> omega ; ///< Particle angular velocity

v2d vel_fluc ; ///< Fluctuating velocity. Should not be externally provided but calculated, using the function *Coarsing::compute_fluc_vel()*
v2d rot_fluc ; ///< Fluctuating angular velocity. Should not be externally provided but calculated, using the function *Coarsing::compute_fluc_vel()*

int Ncf ; ///< Number of contacts
double * id1 ; ///< Index of first particle in contact
double * id2 ; ///< Index of the second particle in contact
vector <double *> pospq ; ///< Location of contact point
vector <double *> lpq ;  ///< Branch vector of the contact, use compute_lpq() to populate this.
vector <double *> fpq; ///< Force at contact
vector <double *> mpq; ///< Moment of particle 1 on 2
vector <double *> mqp ; ///< Moment of particle 2 on 1

// Exta fields if needed
vector<double*> extra ;
vector<std::tuple<std::string, int, int>> extrafields ;

// Some useful functions
int Nnonper=-1 ; ///< Used if additional particles are added as images through the periodic boundary conditions.
int random_test (int N, int Ncf, int d, v2d box ) ; ///< Randomly fill the data structure
int compute_lpq (int d) ; ///< Compute lpq from contact id's and atom locations
int periodic_atoms (int d, v2d bounds, int pbc, v1d Delta, bool omegainclude) ; ///< Copy particles through the periodic boundary conditions. Should call clean_periodic_atoms() after the full coarse-graining computation has been performed to clean the added atoms.
int clean_periodic_atoms () {if (Nnonper==-1) printf("ERR: must call periodic_atoms before cleaning the periodic_atoms\n") ; else N=Nnonper ; return 0 ; } ///< Clean periodic atoms.
bool check_field_availability(string name) ;
int add_extra_field (int length, std::string name)
{
  extrafields.push_back({name, length, extra.size()}) ;
  extra.resize(extra.size()+length) ;
  return std::get<2>(extrafields.back());
}
} ;
//------------------------------------------------------
#include "WindowLibrary.h"

//=========================================================
/** \addtogroup CoarseGraining Coarse-graining */
/** \brief Main Coarse graining class */
class Coarsing
{
public :
    Coarsing (int dd, v1i nnpt, v2d bbox, int T) : cT(0), flags(0)
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
        //grid_neighbour() ;
        set_field_struct() ;
        Window = new LibLucy3D( &data, w, d) ;
    }
    ~Coarsing() { if (Window != nullptr) delete Window ;
                  if (CGPtemp != nullptr) delete CGPtemp ; }

    int d ; ///< Number of dimensions
    int Npt; ///<Number of coarse graining points
    int Time; ///<Total timesteps
    int cT ; ///< Current timestep
    double cutoff ; ///< CG width, and cutoff
    vector <CGPoint> CGP ; ///< List of Coarse Graining points
    vector <CGPoint> * CGPtemp = nullptr ; ///< Temporary cgpoint list that can store temporal averages if needed
    vector <int> npt; ///< Number of points per dimension
    vector <int> nptcum ; ///< Cumulated number of points per dimensions (usefull for quick finding of the closest CG for a grain)
    v1d dx ; ///< Distances between CG points
    v2d box ; ///< CG point location
    LibBase * Window = nullptr ; ///> Pointer to the averaging window


    // Fields variable and function
    unsigned int flags ; ///< Flags deciding which fields to coarse-grain
    vector <string> Fields, Fname ; ///< Flagged field names
    vector <int> Fidx ;  ///< Where the fields is referenced in the fields vector in the CGPoint. -1 if not flagged
    vector <TensorOrder> Ftype ; ///< Flagged field types
    vector <struct Field > FIELDS ; ///< All allowed fields (initialized in grid_getfields)
    int get_id(string nm) ; ///< Find field ID from field name
    struct Field * get_field(string nm) ; ///< Find Field from name
    Pass set_flags (vector <string> s) ; ///< Set the fields which are requested from the coarse-graining

    // Grid functions
    int set_field_struct() ; ///< Set the FIELDS structure, with all the different CG properties that can be computed.
    int add_extra_field(string name, TensorOrder order, FieldType type) ; ///< Used to add extra user-defined fields
    int setWindow (Windows win, double w, vector <bool> per ={}, vector<int> boxes = {}, vector<double> deltas = {}) ; ///< Set the windowing function, calling the templated version
    template <Windows W> int setWindow () ; ///< Set the windowing function
    template <Windows W> int setWindow (double w) ; ///< Set the windowing function
    template <Windows W> int setWindow (double w, vector<bool> per, vector<int> boxes, vector<double> deltas) ; ///< Set the windowing function for the LibLucyND_Periodic (special one...)
    int grid_generate() ; ///< Generate the coarse-graining grid
    int grid_neighbour() ; ///< Generated neighbors in the coarse-graining grid
    std::map<std::string, size_t> grid_setfields() ; ///< Set the fields at each CG point
    vector<FieldType> grid_getfields() ; ///< Extract fields from each CG point
    v2d get_bounds() ; ///< Extract the simulation boundaries
    CGPoint * reverseloop (string type) ; ///< go through the table in reverse order of the dimensions (for the writing phase essentially)
    int find_closest (int id) ; ///< Find the closest CG point to a particle
    int find_closest_pq (int id) ; ///< Find the closest CG point to a contact
    v1d interpolate_vel(int id, bool usetimeavg=false) { return interpolate_vel_nearest (id, usetimeavg) ; } ///< Interpolate the velocity \todo Use something better to interpolate velocity than the nearest neighbor interpolation.
    v1d interpolate_rot(int id, bool usetimeavg=false) { return interpolate_rot_nearest (id, usetimeavg) ; } ///< Interpolate the angular velocity \todo Use something better to interpolate velocity than the nearest neighbor interpolation.
    v1d interpolate_vel_nearest (int id, bool usetimeavg=false) ; ///< Nearest neighbor interpolation for the velocity
    v1d interpolate_rot_nearest (int id, bool usetimeavg=false) ; ///< Nearest neighbor interpolation for the angular velocity
    v1d interpolate_vel_trilinear (int id, bool usetimeavg) ; ///< Tri-linear interpolation (only implemented in 3D, probably not too hard to implement in ND but annoying ...)

    int idx_FastFirst2SlowFirst (int n) ; ///< Change array traversing order

    // Windowing functions
    //double window(double r) {Lucy(r) ; }
    //double window_int (v1d r1, v1d lpq, v1d x) {printf("Numerical integration of wpqf not implemented\n") ; } ///< Numerical integration: not implemented
    //double window_int(double r1, double r2) {return window_avg(r1, r2) ; } ///< Overload to avoid integration ...
    //double window_avg (double r1, double r2) {return (0.5*(Lucy(r1)+Lucy(r2))) ; }
    //double Lucy (double r) {static double cst=105./(16*M_PI*w*w*w) ; if (r>=w) return 0 ; else {double f=r/w ; return (cst*(-3*f*f*f*f + 8*f*f*f - 6*f*f +1)) ; }}
    double normdiff (v1d a, v1d b) {double res=0 ; for (int i=0 ; i<d ; i++) res+=(a[i]-b[i])*(a[i]-b[i]) ; return (sqrt(res)) ; } ; ///< convenience function to to the difference of 2 vectors.
    // Coarse graining functions
    int pass_1 () ; ///< Coarse-grain anything based on particles (not contacts) which does not need fluctuating quantities
    int pass_2 (bool usetimeavg=false) ; ///< Coarse-grain anything based on particles (not contacts) which needs fluctuating quantities (call the compute_fluc_ functions before)
    int pass_3 () ; ///< Coarse-grain anything based on contact informations (no fluctuations).
    int pass_4 () ; ///< Coarse-grain anything based on contact informations & fluctuations.
    int pass_5 () ; ///< Calculation of derived quantities
    int compute_fluc_vel (bool usetimeavg=false) ; ///< Velocity fluctuation computation
    int compute_fluc_rot (bool usetimeavg=false) ; ///< Angular velocity fluctuation computation
    bool hasvelfluct=false, hasrotfluct=false ;

    // Data handling functions

    struct Data data ; ///< Data storage

    // Time and output handling
    int mean_time(bool temporary=false) ; ///< Perform a time average of the coarse-grained data
    int write_vtk(string sout) ; ///< Write CG data as VTK
    int write_netCDF (string sout) ; ///< \deprecated Write CG data as netCDF
    int write_NrrdIO (string path) ; ///< Write CG data as NrrdIO file format
    int write_matlab (string path, bool squeeze = false) ; ///< Write CG data as Matlab file
    int write_numpy (string path, bool squeeze = false) ; ///< Write CG data as numpy files (.npy) ;
    int write_numpy_npy (string path, bool squeeze) ;
    std::pair<size_t, uint8_t*> write_numpy_locbuffer (bool squeeze) {return write_numpy_buffer(-2, squeeze) ; }
    std::pair<size_t, uint8_t*> write_numpy_buffer (int id, bool squeeze) ;
} ;


//-------------------------------------------------------
template <Windows W>
int Coarsing::setWindow ()
{ double w= (*std::min_element(dx.begin(),dx.end())*1) ; // w automatically set
  setWindow<W>(w) ; return 0 ; }
//-------------------------------------------------------
template <Windows W>
int Coarsing::setWindow (double w)
{
  static_assert(W != Windows::LucyND_Periodic) ;
  switch (W) {
      case Windows::Rect3D :
         Window=new LibRect3D (&data, w, d) ;
        break ;
      case Windows::Sphere3DIntersect :
        Window=new LibSphere3DIntersect (&data, w, d) ;
        break ;
      case Windows::SphereNDIntersect :
        Window=new LibSphereNDIntersect (&data, w, d) ;
        break ;
      case Windows::Lucy3D :
        Window=new LibLucy3D (&data, w, d) ;
        break ;
      case Windows::Lucy3DFancyInt :
        Window=new LibLucy3DFancyInt (&data, w, d) ;
        break ;
      case Windows::Hann3D :
        Window=new LibHann3D (&data, w, d) ;
        break ;
      case Windows::RectND :
        Window=new LibRectND (&data, w, d) ;
        break ;
      case Windows::LucyND :
        Window=new LibLucyND (&data, w, d) ;
        break ;
      default:
        printf("Unknown window, check Coarsing::setWindow") ;
  }
  cutoff = Window->cutoff() ;
  printf("Window and cutoff: %g %g \n", w, cutoff) ;
  return 0 ;
}
//-------------------------------------------------------
template <Windows W>
int Coarsing::setWindow (double w, vector<bool> per, vector<int> boxes, vector<double> deltas)
{
  static_assert(W == Windows::LucyND_Periodic) ;
  cutoff = Window->cutoff() ;
  printf("Window and cutoff: %g %g \n", w, cutoff) ;

  int p = 0 ;
  for (size_t i=0 ; i<per.size() ; i++)
    if (per[i])
        p |= (1<<i) ;

  Window = new LibLucyND_Periodic (&data,w,d,p,boxes,deltas) ;
return 0 ;
}

#endif
