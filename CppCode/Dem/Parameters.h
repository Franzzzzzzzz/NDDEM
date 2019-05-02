#ifndef PARAMETERS
#define PARAMETERS

#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <fstream>
#include <map>
#include <experimental/filesystem>
#include "Typedefs.h"
#include "Xml.h"

using namespace std ;
enum class ExportType {NONE=0, CSV=1, VTK=2, NETCDFF=4, XML=8, XMLbase64=16, CSVA=32} ;
enum class ExportData {NONE=0, POSITION=1, VELOCITY=2, OMEGA=4, OMEGAMAG=8, ORIENTATION=16, COORDINATION=32} ;
inline ExportType & operator|=(ExportType & a, const ExportType b) {a= static_cast<ExportType>(static_cast<int>(a) | static_cast<int>(b)); return a ; }
inline ExportData & operator|=(ExportData & a, const ExportData b) {a= static_cast<ExportData>(static_cast<int>(a) | static_cast<int>(b)); return a ; }
inline bool operator& (ExportType & a, ExportType b) {return (static_cast<int>(a) & static_cast<int>(b)) ; }
inline bool operator& (ExportData & a, ExportData b) {return (static_cast<int>(a) & static_cast<int>(b)) ; }
class Parameters {
public :
    Parameters (int dd, int NN):
        d(7),           //spatial dimensions
        N(5),           //number of particles
        tdump(1),       //dump data every these timesteps
        tinfo(100),
        T(5000),        //number of timesteps
        dt(1),          //timestep
        rho(1),         //density (unit [M.L^(-d)]) WARNING UNUSED
        Kn(0.0001) ,    //Hooke stiffness
        Kt(0.00001) ,   //Tangential stiffness
        Gamman(0.0001), //Normal damping
        Gammat(0),      //Tangential damping
        Mu(0.5),        // Friction coefficient
        skin(1.0), skinsqr(1.0),      // Skin size (for verlet list optimisation)
        //dumpkind(ExportType::NONE),    //How to dump: 0=nothing, 1=csv, 2=vtk
        //dumplist(ExportData::POSITION),
        Directory ("Output")
        {
         reset_ND(NN,dd) ;
        }

    void reset_ND (int NN, int dd)
    {
     N=NN ; d=dd ; r.resize(N,0.15) ; //particle size
     m.resize(N, 1e-2); // Set the masses directly
     I.resize(N,1) ; //Momentum of inertia (particles are sphere)
     g.resize(d,0) ; // Initialise gravity
     Frozen.resize(N,false) ;

     Boundaries.resize(d, vector <double> (4,0.0)) ; // Boundary type in [:,3]: 0=regular pbc, 1=wall}
    }

    uint d ; int N, tdump, tinfo ;
    double T ;
    double dt, rho, Kn, Kt, Gamman, Gammat, Mu ;
    double skin, skinsqr ;
    vector <std::pair<ExportType,ExportData>> dumps ;
    //ExportType dumpkind ;
    //ExportData dumplist ;
    vector <double> r, m, I, g ;
    vector <bool> Frozen ;
    vector < vector <double> > Boundaries ;
    string Directory ;
    bool orientationtracking ;

    map<float, string> events ;

// Useful functions
    int set_boundaries() ;
    //int init_particles(v2d & X, v2d & A) ;
    void perform_PBC(v1d & X, u_int32_t & PBCFlags) ;
    int init_mass() ;
    int init_inertia() ;

    void load_datafile (char path[], v2d & X, v2d & V, v2d & Omega) ;
    void check_events(float time, v2d & X, v2d & V, v2d & Omega) ;
    void interpret_command (istream & in, v2d & X, v2d & V, v2d & Omega) ;
    void remove_particle (int idx, v2d & X, v2d & V, v2d & A, v2d & Omega, v2d & F, v2d & FOld, v2d & Torque, v2d & TorqueOld) ;
    void add_particle (v2d & X, v2d & V, v2d & A, v2d & Omega, v2d & F, v2d & FOld, v2d & Torque, v2d & TorqueOld) ;
    void init_locations (char *line, v2d & X) ;

    void display_info(int tint, v2d& V, v2d& Omega, v2d& F, v2d& Torque, int, int) ;
    void quit_cleanly() ;
    void finalise();
    void xml_header () ;
    int dumphandling (int ti, double t, v2d &X, v2d &V, v1d &Vmag, v2d &A, v2d &Omega, v1d &OmegaMag, vector<u_int32_t> &PBCFlags, v1d & Z) ;


// For Xml Writing
    XMLWriter * xmlout ;
} ;

#include "Tools.h"
#endif
