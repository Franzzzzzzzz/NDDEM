#ifndef PARAMETERS
#define PARAMETERS

#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <fstream>
#include "Typedefs.h"
#include "Tools.h"
#include "Xml.h"

using namespace std ;
enum class ExportType {NONE, CSV, VTK, NETCDFF, XML, XMLbase64} ;
class Parameters {
public :
    Parameters (int dd, int NN):
        d(7),           //spatial dimensions
        N(5),           //number of particles
        tdump(1),       //dump data every these timesteps
        tinfo(100),
        dumpkind(ExportType::NONE),    //How to dump: 0=nothing, 1=csv, 2=vtk
        T(5000),        //number of timesteps
        rho(1),         //density (unit [M.L^(-d)]) WARNING UNUSED
        dt(1),          //timestep
        Kn(0.0001) ,    //Hooke stiffness
        Kt(0.00001) ,   //Tangential stiffness
        Gamman(0.0001), //Normal damping
        Gammat(0),      //Tangential damping
        Mu(0.5)         // Friction coefficient
        {
         reset_ND(NN,dd) ;
         if (dumpkind==ExportType::XML || dumpkind==ExportType::XMLbase64)
             xmlout= new XMLWriter("dump.xml") ;
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

    unsigned int d ;
    int N, tdump, tinfo ;
    ExportType dumpkind ;
    double T ;
    double rho, dt, Kn, Kt, Gamman, Gammat, Mu ;
    vector <double> r, m, I, g ;
    vector <bool> Frozen ;
    vector < vector <double> > Boundaries ;

// Useful functions
    int set_boundaries() ;
    //int init_particles(v2d & X, v2d & A) ;
    void perform_PBC(v1d & X) ;
    int init_mass() ;
    int init_inertia() ;

    void load_datafile (char path[], v2d & X, v2d & V, v2d & Omega) ;
    void remove_particle (int idx, v2d & X, v2d & V, v2d & A, v2d & Omega, v2d & F, v2d & FOld, v2d & Torque, v2d & TorqueOld) ;
    void add_particle (v2d & X, v2d & V, v2d & A, v2d & Omega, v2d & F, v2d & FOld, v2d & Torque, v2d & TorqueOld) ;
    void init_locations (char *line, v2d & X) ;

    void display_info(int tint, v2d& V, v2d& Omega, v2d& F, v2d& Torque, int, int) ;

// For Xml Writing
    XMLWriter * xmlout ;
} ;

#endif
