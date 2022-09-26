#ifndef PARAMETERS
#define PARAMETERS

/** \addtogroup DEM
 *  @{ */

#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <fstream>
#include <map>
#include <boost/variant.hpp>
#include <filesystem>
#include "Typedefs.h"
#include "Xml.h"
#include "Tools.h"
#include "RigidBody.h"
//
#include <boost/random.hpp>

using namespace std ;
enum class ExportType {NONE=0, CSV=1, VTK=2, NETCDFF=4, XML=8, XMLbase64=16, CSVA=32, CSVCONTACT=64} ;
enum class ExportData {NONE=0, POSITION=1, VELOCITY=2, OMEGA=4, OMEGAMAG=8, ORIENTATION=16, COORDINATION=32, RADIUS=64, IDS=128, FN=256, FT=512, TORQUE=1024, GHOSTMASK=2048, GHOSTDIR=4096, BRANCHVECTOR=8192} ; ///< Flags for export data control
enum class WallType {PBC=0, WALL=1, MOVINGWALL=2, SPHERE=3, ROTATINGSPHERE=4, PBC_LE=5} ; ///< Wall types
inline ExportType & operator|=(ExportType & a, const ExportType b) {a= static_cast<ExportType>(static_cast<int>(a) | static_cast<int>(b)); return a ; }
inline ExportData & operator|=(ExportData & a, const ExportData b) {a= static_cast<ExportData>(static_cast<int>(a) | static_cast<int>(b)); return a ; }
inline bool operator& (ExportType & a, ExportType b) {return (static_cast<int>(a) & static_cast<int>(b)) ; }
inline bool operator& (ExportData & a, ExportData b) {return (static_cast<int>(a) & static_cast<int>(b)) ; }

template <int d>
class Multiproc ;
/** \brief Generic class to handle the simulation set up
 *
 */
template <int d>
class Parameters {
public :
    Parameters () {Parameters(0) ; }
    Parameters (int NN):
        N(5),           //number of particles
        tdump(100000000),       //dump data every these timesteps
        tinfo(100),
        T(5000),        //number of timesteps
        dt(0.0001),          //timestep
        rho(1),         //density (unit [M.L^(-d)]) WARNING UNUSED
        Kn(0.0001) ,    //Hooke stiffness
        Kt(0.00001) ,   //Tangential stiffness
        Gamman(0.0001), //Normal damping
        Gammat(0),      //Tangential damping
        Mu(0.5),        // Friction coefficient
        Mu_wall(0.5),        // Wall friction coefficient
        damping(0.0),
        skin(1.0), skinsqr(1.0),      // Skin size (for verlet list optimisation)
        //dumpkind(ExportType::NONE),    //How to dump: 0=nothing, 1=csv, 2=vtk
        //dumplist(ExportData::POSITION),
        Directory ("Output"),
        orientationtracking(true),
        wallforcecompute(false),
        wallforcerequested(false),
        wallforcecomputed(false)
        {
         reset_ND(NN) ;
        } ///< Set the default values for all parameters. Calls to setup parameter function should be provided after initialisation of this class.

    void reset_ND (int NN)
    {
     N=NN ; r.resize(N,0.5) ; //particle size
     m.resize(N, 1e-2); // Set the masses directly
     I.resize(N,1) ; //Momentum of inertia (particles are sphere)
     g.resize(d,0) ; // Initialise gravity
     Frozen.resize(N,false) ;

     Boundaries.resize(d, vector <double> (4,0.0)) ; // Boundary type in [:,3]: 0=regular pbc, 1=wall}
    }///< reset the full simulation.

    int N; ///< Number of particles
    int tdump ; ///< Write dump file every this many timesteps
    int tinfo ; ///< Show detail information on scren every this many timesteps
    double T ; ///< Run until this time (note it is a floating point).
    double dt ; ///< timestep
    double rho; ///< density
    double Kn ; ///< Normal spring constant
    double Kt ; ///< Tangential spring constant
    double Gamman; ///< Normal dissipation
    double Gammat ; ///< Tangential dissipation
    double Mu ; ///< Fricton
    double Mu_wall; //< Wall friction
    double damping; //< Artificial rolling damping
    double skin ; ///< Skin for use in verlet list \warning Experimental
    double skinsqr ; ///< Skin squared for use in verlet list \warning Experimental
    ContactModels ContactModel=HOOKE ;
    vector <std::pair<ExportType,ExportData>> dumps ; ///< Vector linking dump file and data dumped
    //ExportType dumpkind ;
    //ExportData dumplist ;
    vector <double> r ; ///< Particle radii
    vector <double> m ; ///< Particle mass
    vector <double> I ; ///< Particule moment of inertia
    vector <double> g ; ///< Gravity vector
    vector <bool> Frozen ; ///< Frozen atom if true
    vector < vector <double> > Boundaries ; ///< List of boundaries. Second dimension is {min, max, length, type}.
    string Directory ; ///< Saving directory
    bool orientationtracking ; ///< Track orientation?
    bool wallforcecompute ; ///< Compute for on the wall?
    bool wallforcerequested ; ///< Compute for on the wall?
    bool wallforcecomputed ; ///< Compute for on the wall?
    bool contactforcedump ; ///< Extract the forces between grains as well?
    unsigned long int seed = 5489UL ; ///< Seed for the boost RNG. Initialised with the default seed of the Mersenne twister in Boost
    RigidBodies_<d> RigidBodies ; ///< Handle all the rigid bodies
    
    multimap<float, string> events ; ///< For storing events. first is the time at which the event triggers, second is the event command string, parsed on the fly when the event gets triggered.

// Useful functions
    int set_boundaries() ;  ///< Set default boundaries
    //int init_particles(v2d & X, v2d & A) ;
    void perform_PBC(v1d & X, uint32_t & PBCFlags) ; ///< Bring particle back in the simulation box if the grains cross the boundaries
    void perform_PBCLE_move() ;
    void perform_PBCLE (v1d & X, v1d & V, uint32_t & PBCFlag) ;

    void perform_MOVINGWALL() ; ///< Move the boundary wall if moving.
    int init_mass() ; ///< Initialise particle mass
    int init_inertia() ; ///< Initialise particle moment of inertia

    void do_nothing (double time __attribute__((unused))) {return;}
    double grav_intensity = 10, grav_omega=1 ; int grav_rotdim[2]={0,1} ;
    void do_gravityrotate (double time) {g[grav_rotdim[0]]=grav_intensity*cos(time*grav_omega); g[grav_rotdim[1]]=grav_intensity*sin(time*grav_omega); return;}
    void (Parameters::*update_gravity) (double time) = &Parameters::do_nothing ;

    void load_datafile (char path[], v2d & X, v2d & V, v2d & Omega) ; ///< Load and parse input script
    void check_events(float time, v2d & X, v2d & V, v2d & Omega) ; ///< Verify if an event triggers at the current time time.
    void interpret_command (istream & in, v2d & X, v2d & V, v2d & Omega) ; ///< Parse input script commands
    void interpret_command (string & in, v2d & X, v2d & V, v2d & Omega) ; ///< Parse input script commands

    void remove_particle (int idx, v2d & X, v2d & V, v2d & A, v2d & Omega, v2d & F, v2d & FOld, v2d & Torque, v2d & TorqueOld) ; ///< Not tested. \warning not really tested
    void add_particle (/*v2d & X, v2d & V, v2d & A, v2d & Omega, v2d & F, v2d & FOld, v2d & Torque, v2d & TorqueOld*/) ; ///< Not implemented
    void init_locations (char *line, v2d & X) ; ///< Set particle locations
    void init_radii (char line[], v1d & r) ;  ///< Set particle radii

    void display_info(int tint, v2d& V, v2d& Omega, v2d& F, v2d& Torque, int, int) ; ///< On screen information display
    void quit_cleanly() ; ///< Close opened dump files in the event of an emergency quit (usually a SIGINT signal to the process)
    void finalise(); ///< Close opened dump files
    void xml_header () ; ///< Write the Xml header (should go into a file dedicated to the writing though ...)
    int dumphandling (int ti, double t, v2d &X, v2d &V, v1d &Vmag, v2d &A, v2d &Omega, v1d &OmegaMag, vector<uint32_t> &PBCFlags, v1d & Z, Multiproc<d> & MP) ; ///< Dump writing functions
    int savecsvcontact (FILE * out, ExportData outflags, Multiproc<d> & MP, cv2d & X)
    {
     if (outflags & ExportData::IDS) fprintf(out, "id_i, id_j, ") ;
     if (outflags & ExportData::POSITION)
     {
         for (int dd = 0 ; dd<d ; dd++)
             fprintf(out, "x%d_i, ", dd) ;
         for (int dd = 0 ; dd<d ; dd++)
             fprintf(out, "x%d_j, ", dd) ;
     }
     if (outflags & ExportData::FN)
         for (int dd = 0 ; dd<d ; dd++)
             fprintf(out, "Fn%d_i, ", dd) ;
     if (outflags & ExportData::FT)
         for (int dd = 0 ; dd<d ; dd++)
             fprintf(out, "Ft%d_i, ", dd) ;
     if (outflags & ExportData::TORQUE)
     {
         for (int dd = 0 ; dd<d*(d-1)/2 ; dd++)
         {
             auto val = Tools<d>::MASIndex[dd] ;
             fprintf(out, "Torque%d:%d_i, ", val.first, val.second) ;
         }
         for (int dd = 0 ; dd<d*(d-1)/2 ; dd++)
         {
             auto val =  Tools<d>::MASIndex[dd] ;
             fprintf(out, "Torque%d:%d_j, ", val.first, val.second) ;
         }
     }
     if (outflags & ExportData::BRANCHVECTOR)
     {
         for (int dd = 0 ; dd<d ; dd++)
             fprintf(out, "lij%d, ", dd) ;
     }
     if (outflags & ExportData::GHOSTMASK)
        fprintf(out, "GhostMask, ") ;
     if (outflags & ExportData::GHOSTDIR)
        fprintf(out, "GhostDir, ") ;

     fseek (out, -2, SEEK_CUR) ;
     fprintf(out, "\n") ;

     for (auto & CLp : MP.CLp)
     {
         for (auto & contact: CLp.v)
         {
             if (outflags & ExportData::IDS)
                fprintf(out, "%d %d ", contact.i, contact.j) ;
             if (outflags & ExportData::POSITION)
             {
                 for (auto dd : X[contact.i])
                  fprintf(out, "%g ", dd) ;
                 for (auto dd : X[contact.j])
                  fprintf(out, "%g ", dd) ;
             }
             if (outflags & ExportData::FN)
                for (auto dd : contact.infos->Fn)
                  fprintf(out, "%g ", dd) ;
             if (outflags & ExportData::FT)
                for (auto dd : contact.infos->Ft)
                  fprintf(out, "%g ", dd) ;

             if (outflags & ExportData::TORQUE)
             {
                for (auto dd : contact.infos->Torquei)
                  fprintf(out, "%g ", dd) ;
                for (auto dd : contact.infos->Torquej)
                  fprintf(out, "%g ", dd) ;
             }

             if (outflags & ExportData::GHOSTMASK)
                 fprintf(out, "%d ", contact.ghost) ;
             if (outflags & ExportData::GHOSTDIR)
                 fprintf(out, "%d ", contact.ghostdir) ;

             if (outflags & ExportData::BRANCHVECTOR)
             {
                auto [loc,branch] = contact.compute_branchvector(X,Boundaries, d) ; 
               
                for (int dd = 0 ; dd<d ; dd++) fprintf(out, "%g ", branch[dd]) ;
             }
         fprintf(out, "\n") ;
         }
     }
     return 0 ;
    }


// For Xml Writing
    XMLWriter * xmlout ;
} ;
/** @}*/

/*****************************************************************************************************
 *                                                                                                   *
 *                                                                                                   *
 *                                                                                                   *
 * IMPLEMENTATIONS                                                                                   *
 *                                                                                                   *
 *                                                                                                   *
 *                                                                                                   *
 * ***************************************************************************************************/
double nan_or_double (istream & in)
{
  char c; std::string word ; 
  if (!in.good()) return std::numeric_limits<double>::quiet_NaN();
  while (isspace(c = in.peek())) in.get();
  if ( !isdigit(c) && c!='-' && c!='.') { in >> word ; return std::numeric_limits<double>::quiet_NaN(); }
  double x ; 
  in >> x;
  return x ; 
}


/// Some Infos on set_boundaries
template <int d>
int Parameters<d>::set_boundaries()
{
    for (int i=0 ; i<d ; i++)
    {
        Boundaries[i][0]= 0 ;
        Boundaries[i][1]= 1 ;
        Boundaries[i][2]=Boundaries[i][1]-Boundaries[i][0] ; //Precomputed to increase speed
        Boundaries[i][3]=static_cast<int>(WallType::PBC) ; // PBC by default
    }
    //np.savetxt('Boundaries.csv', Boundaries , delimiter=',', fmt='%.6g', header='Low,High,Size,Type', comments='')
    return 0 ;
}
//----------------------------------------------------
template <int d>
void Parameters<d>::perform_PBC (v1d & X, uint32_t & PBCFlag)
{
 for (int j=0 ; j<d ; j++)
 {
   if (Boundaries[j][3]==static_cast<int>(WallType::PBC)) //PBC
   {
    if      (X[j]<Boundaries[j][0]) {X[j] += Boundaries[j][2] ; PBCFlag |= (1<<j) ;}
    else if (X[j]>Boundaries[j][1]) {X[j] -= Boundaries[j][2] ; PBCFlag |= (1<<j) ;}
   }
 }
}
//----------------------------------------------------
template <int d>
void Parameters<d>::perform_PBCLE (v1d & X, v1d & V, uint32_t & PBCFlag)
{
    if (Boundaries[0][3]==static_cast<int>(WallType::PBC_LE))
    {
        if      (X[0]<Boundaries[0][0])
        {
            X[0] += Boundaries[0][2] ; PBCFlag |= 1 ;
            X[1] += Boundaries[0][5] ;
            V[1] += Boundaries[0][2]*Boundaries[0][4] ; ;
        }
        else if (X[0]>Boundaries[0][1])
        {
            X[0] -= Boundaries[0][2] ; PBCFlag |= 1 ;
            X[1] -= Boundaries[0][5] ;
            V[1] -= Boundaries[0][2]*Boundaries[0][4] ; ;
        }

        if ( X[1]<Boundaries[1][0]) {X[1]+=Boundaries[1][2] ; PBCFlag |= 2 ; }
        if ( X[1]>Boundaries[1][1]) {X[1]-=Boundaries[1][2] ; PBCFlag |= 2 ; }
    }
}
//----------------------------------------------------
template <int d>
void Parameters<d>::perform_PBCLE_move ()
{
    if (Boundaries[0][3]==static_cast<int>(WallType::PBC_LE))
    {
        Boundaries[0][5] += dt*Boundaries[0][4]*Boundaries[0][2] ;
        if (Boundaries[0][5]>Boundaries[1][1])
            Boundaries[0][5] -= Boundaries[1][2] ;
    }
}
//----------------------------------------------------
template <int d>
void Parameters<d>::perform_MOVINGWALL ()
{
 for (int j=0 ; j<d ; j++)
 {
  if (Boundaries[j][3]==static_cast<int>(WallType::MOVINGWALL))
  {
    Boundaries[j][0] += Boundaries[j][4] * dt ;
    Boundaries[j][1] += Boundaries[j][5] * dt ;
  }
 }
}
//-----------------------------------------------------
/*int Parameters::init_particles(v2d & X, v2d & A)
{
 boost::random::mt19937 rng;
 boost::random::uniform_01<boost::mt19937> rand(rng) ;
 for (int i=0 ; i<N ; i++)
   for (int dd=0 ; dd<d ; dd++)
   {
     X[i][dd]=rand()*(Boundaries[dd][2]-Boundaries[dd][3]*2*r[i])+(Boundaries[dd][0]+Boundaries[dd][3]*r[i]);//avoid too much overlap with walls
     //m[i]=volume(r[i])*rho ;

   }
 //X[0][5]=0.14 ;
 //V[0][:]=np.zeros((1,d))+0.0001*unitvec(0) ;
 //X[1][:]=np.ones((1,d))*0.7 ; X[1,5]=0.2 ; X[1,4]=0.71 ;
 //X[1][:]=np.array([0.21,0.21,0.9,0.2]) ;
 return 0 ;
}*/
template <int d>
int Parameters<d>::init_mass ()
{
 for (int i=0 ; i<N ; i++)
   m[i]=rho*Tools<d>::Volume(r[i]) ;
return 0 ;
}

template <int d>
int Parameters<d>::init_inertia ()
{
 for (int i=0; i<N ; i++)
 {
   I[i]=Tools<d>::InertiaMomentum (r[i], rho) ;
 }
 return 0 ;
}
//---------------------------------------------------
template <int d>
void Parameters<d>::load_datafile (char path[], v2d & X, v2d & V, v2d & Omega)
{
  ifstream in ;

  in.open(path) ;
  if (!in.is_open()) { printf("[Input] file cannot be open\n"); return ;}

  while (! in.eof())
  {
    interpret_command(in, X, V, Omega) ;
  }

  for (auto v : dumps)
    if (v.first==ExportType::XML || v.first==ExportType::XMLbase64)
      xmlout= new XMLWriter(Directory+"/dump.xml") ;

  in.close() ;
  // Self copy :)
  std::filesystem::path p (path) ;
  std::filesystem::path pcp (Directory+"/in") ;// pcp/= p.filename() ;
  copy_file(p,pcp,std::filesystem::copy_options::overwrite_existing);
}
//-------------------------------------------------
template <int d>
void Parameters<d>::check_events(float time, v2d & X, v2d & V, v2d & Omega)
{
  while (events.size()>0 && events.begin()->first < time)
  {
    stringstream command ; command.str(events.begin()->second) ;
    printf("\nThe following event is implemented now: %s\n", events.begin()->second.c_str()) ;
    interpret_command(command, X,V,Omega) ;
    events.erase(events.begin()) ;
  }

}
//------------------------------------------------------
std::istream& operator>>(std::istream& in, boost::variant<int&,double&,bool&> v)
{
  switch(v.which())
  {
    case 0: in >> boost::get<int&>(v) ; break ;
    case 1: in >> boost::get<double&>(v); break ;
    case 2: in >> boost::get<bool&>(v); break ;
  }
  return(in) ;}
//------------------------------------------------------
template <int d>
void Parameters<d>::interpret_command (string &in, v2d & X, v2d & V, v2d & Omega)
{
    stringstream B(in) ;
    return (interpret_command(B, X, V, Omega)) ;
}
template <int d>
void Parameters<d>::interpret_command (istream & in, v2d & X, v2d & V, v2d & Omega)
{
  map <string, boost::variant<int&,double&,bool&>> SetValueMap = {
     {"Kn",Kn},
     {"Kt",Kt},
     {"GammaN",Gamman},
     {"GammaT",Gammat},
     {"rho",rho},
     {"Mu",Mu},
     {"Mu_wall",Mu_wall},
     {"damping",damping},
     {"T",T},
     {"tdump",tdump},
     {"orientationtracking",orientationtracking},
     {"tinfo",tinfo},
     {"dt",dt},
     {"skin", skin},
  } ;

// Lambda definitions
 auto discard_line = [&](){char line [5000] ; in.getline(line, 5000) ; } ;
 auto read_event = [&](){float time ; char line [5000] ; in >> time ; in.getline (line, 5000) ; events.insert(make_pair(time,line)) ; printf("[INFO] Registering an event: %g -> %s\n", time, line) ;} ;
 auto doauto = [&]() { char line [5000] ; in>>line ;
   if (!strcmp(line, "mass")) init_mass() ;
   else if (!strcmp(line, "rho"))
   {
     rho= m[0]/Tools<d>::Volume(r[0]) ;
     printf("[INFO] Using first particle mass to set rho: %g [M].[L]^-%d\n", rho, d) ;
   }
   else if (!strcmp(line, "inertia")) init_inertia() ;
   else if (!strcmp(line, "location"))
   {
     in >> line ;
     init_locations(line, X) ;
     printf("[INFO] Set all particle locations\n") ;
   }
   else if (!strcmp(line, "radius"))
   {
     in.getline(line, 5000) ;
     init_radii(line, r) ;
     printf("[INFO] Set all particle radii\n") ;
   }
   else if (!strcmp(line, "skin"))
   {
     skin = 2 * *std::max_element(r.begin(), r.end()) ;
     skinsqr = skin*skin ;
   }
   else printf("[WARN] Unknown auto command in input script\n") ;
   printf("[INFO] Doing an auto \n") ;
   } ;
 auto setvalue = [&]() { char line[5000] ; in >> line ;
   try {
     in >> SetValueMap.at(line) ;
     if (!strcmp(line, "skin"))
     {
       double maxradius = *std::max_element(r.begin(), r.end()) ;

       if (skin<maxradius)
         {skin=2*maxradius ; printf("The skin cannot be smaller than the radius") ; }
       skinsqr=skin*skin ;
     }
   }
   catch (const std::out_of_range & e) {
     if (!strcmp(line, "dumps"))
     {
       string word ;
       in>>word ;
       ExportType dumpkind=ExportType::NONE ;
       if (word=="CSV") dumpkind = ExportType::CSV ;
       else if (word=="VTK") dumpkind = ExportType::VTK ;
       else if (word=="NETCDFF") dumpkind = ExportType::NETCDFF ;
       else if (word=="XML") dumpkind = ExportType::XML ;
       else if (word=="XMLbase64") dumpkind = ExportType::XMLbase64 ;
       else if (word=="CSVA") dumpkind = ExportType::CSVA ;
       else if (word=="CONTACTFORCES") {dumpkind = ExportType::CSVCONTACT ; contactforcedump = true ; }
       else if (word=="WALLFORCE") {wallforcecompute = true ; goto LABEL_leave ;} //Jumps at the end of the section
       else {printf("Unknown dump type\n") ; }

       { // New section so g++ doesn't complains about the goto ...
       in>>word ;
       if (word != "with") printf("ERR: expecting keyword 'with'\n") ;
       int nbparam ;
       ExportData dumplist = ExportData::NONE ;
       in>>nbparam ;
       for (int i=0 ; i<nbparam ; i++)
       {
         in>>word ;
         if (word=="Position") dumplist |= ExportData::POSITION ;
         else if (word =="Velocity") dumplist |= ExportData::VELOCITY ;
         else if (word =="Omega") dumplist |= ExportData::OMEGA ;
         else if (word =="OmegaMag") dumplist |= ExportData::OMEGAMAG ;
         else if (word =="Orientation")
         {
           orientationtracking=true ;
           dumplist |= ExportData::ORIENTATION ;
         }
         else if (word =="Coordination") dumplist |= ExportData::COORDINATION ;
         else if (word =="Radius") dumplist |= ExportData::RADIUS ;
         else if (word =="Ids") dumplist |= ExportData::IDS ;
         else if (word =="Fn") dumplist |= ExportData::FN ;
         else if (word =="Ft") dumplist |= ExportData::FT ;
         else if (word =="Torque") dumplist |= ExportData::TORQUE ;
         else if (word =="Branch") dumplist |= ExportData::BRANCHVECTOR ;
         else printf("Unknown asked data %s\n", word.c_str()) ;
       }

       dumps.push_back(make_pair(dumpkind,dumplist)) ;
       }
       LABEL_leave: ; // Goto label (I know, not beautiful, but makes sense here really)
     }
     else
       printf("[ERROR] Unknown parameter to set\n") ;
   }
 } ; // END of the setvalue lambda

// Function mapping
 map<string, function<void()>> Lvl0 ;
 Lvl0["event"] = read_event ;
 Lvl0["CG"] = discard_line ; // Discard CoarseGraining functions in DEM
 Lvl0["set"] = setvalue ;
 Lvl0["auto"] = doauto ;

 Lvl0["directory"] = [&](){in>>Directory ; if (! std::filesystem::exists(Directory)) std::filesystem::create_directory(Directory); };
 Lvl0["dimensions"] = [&](){int nn; int dd ; in>>dd>>nn ; if (N!=nn || d!=dd) {printf("[ERROR] Dimension of number of particles not matching the input file requirements d=%d N=%d\n", d, N) ; std::exit(2) ; }} ;
 Lvl0["location"] = [&](){int id ; in>>id ; for (int i=0 ; i<d ; i++) {in >> X[id][i] ;} printf("[INFO] Changing particle location.\n") ; } ;
 Lvl0["velocity"] = [&](){int id ; in>>id ; for (int i=0 ; i<d ; i++) {in >> V[id][i] ;} printf("[INFO] Changing particle velocity.\n") ; } ;
 Lvl0["omega"]    = [&](){int id ; in>>id ; for (int i=0 ; i<d*(d-1)/2 ; i++) {in >> Omega[id][i] ;} printf("[INFO] Changing particle angular velocity.\n") ; } ;
 Lvl0["freeze"]   = [&](){int id ; in>>id ; Frozen[id]=true ; printf("[INFO] Freezing particle.\n") ;} ;
 Lvl0["radius"]   = [&](){int id ; double radius ; in>>id>>radius ; if(id==-1) r = v1d(N,radius) ; else r[id]=radius ; printf("[INFO] Set radius of particle.\n") ;} ;
 Lvl0["mass"]     = [&](){int id ; double mass ;   in>>id>>mass   ; if(id==-1) m = v1d(N,mass  ) ; else r[id]=mass   ; printf("[INFO] Set mass of particle.\n") ;} ;
 Lvl0["gravity"]  = [&](){for (int i=0 ; i<d ; i++) {in >> g[i] ;} printf("[INFO] Changing gravity.\n") ; } ;
 Lvl0["ContactModel"] = [&](){ std::string s ; in >> s ; if (s=="Hertz") {ContactModel=ContactModels::HERTZ ;printf("[INFO] Using Hertz model\n") ;} else ContactModel=ContactModels::HOOKE ; } ;
 Lvl0["gravityangle"] = [&](){ double intensity, angle ; in >> intensity >> angle ;
    Tools<d>::setzero(g) ;
    g[0] = -intensity * cos(angle / 180. * M_PI) ;
    g[1] = intensity * sin(angle / 180. * M_PI) ;
    printf("[INFO] Changing gravity angle in degree between x0 and x1.\n") ;
    } ;
 Lvl0["gravityrotate"] = [&] () {
     update_gravity =&Parameters::do_gravityrotate;
     Tools<d>::setzero(g) ;
     in >> grav_intensity >> grav_omega >> grav_rotdim[0] >> grav_rotdim[1] ;
     printf("[INFO] Setting up a rotating gravity\n") ;
    } ;
 Lvl0["boundary"] = [&](){size_t id ; in>>id ;
    if (id>=Boundaries.size()) {Boundaries.resize(id+1) ; Boundaries[id].resize(4,0) ; }
    char line [5000] ; in>>line ;
    if (!strcmp(line, "PBC")) Boundaries[id][3]=static_cast<int>(WallType::PBC) ;
    else if (!strcmp(line, "WALL")) Boundaries[id][3]=static_cast<int>(WallType::WALL) ;
    else if (!strcmp(line, "MOVINGWALL")) {Boundaries[id][3]=static_cast<int>(WallType::MOVINGWALL) ; Boundaries[id].resize(4+2, 0) ; }
    else if (!strcmp(line, "SPHERE")) {Boundaries[id][3]=static_cast<int>(WallType::SPHERE) ; Boundaries[id].resize(4+d,0) ; }
    else if (!strcmp(line, "ROTATINGSPHERE")) {Boundaries[id][3]=static_cast<int>(WallType::ROTATINGSPHERE) ; Boundaries[id].resize(4+d+d*(d-1)/2,0) ; }
    else if (!strcmp(line, "PBCLE")) {Boundaries[id][3]=static_cast<int>(WallType::PBC_LE) ; Boundaries[id].resize(6,0) ; }
    else printf("[WARN] Unknown boundary condition, unchanged.\n") ;
    in >> Boundaries[id][0] ; in>> Boundaries[id][1] ;
    Boundaries[id][2]=Boundaries[id][1]-Boundaries[id][0] ;
    if  (Boundaries[id][3] == static_cast<int>(WallType::MOVINGWALL))
    {in >> Boundaries[id][4] ; in >> Boundaries[id][5] ;}
    else if (Boundaries[id][3] == static_cast<int>(WallType::SPHERE) )
    {
            Boundaries[id][4]=Boundaries[id][1] ;
            Boundaries[id][1] = Boundaries[id][0]*Boundaries[id][0] ; // Computing Rsqr for speed
            for (int i=1 ; i<d; i++) // dim 0 has already been read and put in [4]
                in>>Boundaries[id][4+i] ;
    }
    else if (Boundaries[id][3] == static_cast<int>(WallType::ROTATINGSPHERE) )
    {
            Boundaries[id][4]=Boundaries[id][1] ;
            Boundaries[id][1] = Boundaries[id][0]*Boundaries[id][0] ; // Computing Rsqr for speed
            for (int i=1 ; i<d; i++) in>>Boundaries[id][4+i] ;
            for (int i=0; i<d*(d-1)/2 ; i++) in >> Boundaries[id][4+d+i] ;
    }
    else if (Boundaries[id][3] == static_cast<int>(WallType::PBC_LE))
    {
        assert((id==0)) ;
        in >> Boundaries[id][4] ;
        Boundaries[id][5] = 0 ;
    }
    printf("[INFO] Changing BC.\n") ;
   };
 Lvl0["rigid"] = [&] () 
 {
   size_t id, npart ; in >> id ;
   if (RigidBodies.RB.size()<id+1) RigidBodies.RB.resize(id+1) ; 
   string s ; in>>s ; 
   
   if (s=="set")
   {
      in >> npart ; 
      std::vector<int> ids ; int tmp ; 
      for (size_t i=0 ; i<npart ; i++)
      {
        in >> tmp ; 
        ids.push_back(tmp) ; 
      }
      RigidBodies.RB[id].setparticles(ids, X, m) ; 
   }
   else if (s=="gravity")
   {
     in >> s ;
     if (s=="on") RigidBodies.RB[id].cancelgravity = false ; 
     else if (s=="off") RigidBodies.RB[id].cancelgravity = true ;
   }
   else if (s=="addforce")
   {
     for (int dd=0 ; dd<d ; dd++)
       in >> RigidBodies.RB[id].addforce[dd] ; //= nan_or_double(in) ; 
   }
   else if (s=="velocity")
   {
     for (int dd=0 ; dd<d ; dd++)
       RigidBodies.RB[id].setvel[dd] = nan_or_double(in) ;
     printf("%g %g\n", RigidBodies.RB[0].setvel[0], RigidBodies.RB[0].setvel[1]) ; 
   }
   
   printf("[INFO] Defining a rigid body.\n") ;
 };

// Processing
 string line ;
 in>>line;
 if (line[0]=='#') {discard_line() ; return  ;}
 try {Lvl0.at(line)() ; }
 catch (const std::out_of_range & e) { printf("LVL0 command unknown: %s\n", line.c_str()) ; discard_line() ; }

}
//================================================================================================================================================

//=====================================
template <int d>
void Parameters<d>::remove_particle (int idx, v2d & X, v2d & V, v2d & A, v2d & Omega, v2d & F, v2d & FOld, v2d & Torque, v2d & TorqueOld)
{
    printf("WARN: it is probably a bad idea to use remove particle which hasn't been tested ... \n") ;
    X.erase (X.begin()+idx) ; V.erase (V.begin()+idx) ;
    A.erase (A.begin()+idx) ; Omega.erase (Omega.begin()+idx) ;
    F.erase (F.begin()+idx) ; FOld.erase (FOld.begin()+idx) ;
    Torque.erase (Torque.begin()+idx) ; TorqueOld.erase (TorqueOld.begin()+idx) ;
    I.erase(I.begin()+idx) ; m.erase(m.begin()+idx) ; r.erase(r.begin()+idx) ;
    Frozen.erase(Frozen.begin()+idx) ;
    N-- ;
}
//--------------------
template <int d>
void Parameters<d>::add_particle (/*v2d & X, v2d & V, v2d & A, v2d & Omega, v2d & F, v2d & FOld, v2d & Torque, v2d & TorqueOld*/)
{
    printf("add_particle NOT IMPLEMENTED YET\n") ;
}
//----------------
template <int d>
void Parameters<d>::init_locations (char *line, v2d & X)
{
    boost::random::mt19937 rng(seed);
    boost::random::uniform_01<boost::mt19937> rand(rng) ;
    if (!strcmp(line, "square"))
    {
        auto m = *(std::max_element(r.begin(), r.end())) ;
        printf("%g\n", m) ;
        int dd ;
        for (dd=0 ; dd<d ; dd++) X[0][dd]=Boundaries[dd][0]+m ;
        for (int i=1 ; i<N ; i++)
        {
            X[i]=X[i-1] ;
            for (dd=0 ; dd<d ; dd++)
            {
                X[i][dd] += 2*m ;
                if (X[i][dd]>Boundaries[dd][1]-m)
                    X[i][dd] = Boundaries[dd][0]+m ;
                else
                    break ;
            }
            if (dd==d) {printf("WARN: cannot affect all particles on the square lattice, not enough space in the simulation box\n") ; break ; }
        }
    }
    else if (!strcmp(line, "randomsquare"))
    {
        auto m = *(std::max_element(r.begin(), r.end())) ;
        printf("%g\n", m) ;
        int dd ;
        for (dd=0 ; dd<d ; dd++) X[0][dd]=Boundaries[dd][0]+m ;
        for (int i=1 ; i<N ; i++)
        {
            X[i]=X[i-1] ;
            for (dd=0 ; dd<d ; dd++)
            {
                X[i][dd] += 2*m ;
                if (X[i][dd]>Boundaries[dd][1]-m)
                    X[i][dd] = Boundaries[dd][0]+m + 0.1*m*(rand()-0.5) ;
                else
                    break ;
            }
            if (dd==d) {printf("WARN: cannot affect all particles on the square lattice, not enough space in the simulation box\n") ; break ; }
        }
    }
    else if (!strcmp(line, "randomdrop"))
    {

        for (int i=0 ; i<N ; i++)
        {
         for(int dd=0 ; dd < d ; dd++)
         {
           if (Boundaries[dd][3]==0)
             X[i][dd] = rand()*Boundaries[dd][2] + Boundaries[dd][0] ;
           else
             X[i][dd] = rand()*(Boundaries[dd][2]-2*r[i]) + Boundaries[dd][0] + r[i] ;
         }
        }

    }
    else if (!strcmp(line, "insphere"))
    {
        printf("Location::insphere assumes that wall #d is a sphere") ; fflush(stdout) ;

        for (int i=0 ; i<N ; i++)
        {
         double dst=0 ;
         printf(".") ; fflush(stdout);
         do {
            dst=0 ;
            for(int dd=0 ; dd < d ; dd++)
            {
              X[i][dd] = (rand()*Boundaries[d][0]*2-Boundaries[d][0]) + Boundaries[d][4+dd] ;
              dst += (Boundaries[d][4+dd]-X[i][dd])*(Boundaries[d][4+dd]-X[i][dd]) ;
            }
         } while ( sqrt(dst) > Boundaries[d][0]-r[i]) ;
        }
    }
    else if (!strcmp(line, "roughinclineplane"))
    {
      printf("Location::roughinclineplane assumes a plane of normal [1,0,0...] at location 0 along the 1st dimension.") ; fflush(stdout) ;
      auto m = *(std::max_element(r.begin(), r.end())) ; // Max radius
      double delta=0.1*m ;
      for (int dd=0 ; dd<d ; dd++) X[0][dd]=Boundaries[dd][0]+m+delta ;
      Frozen[0]=true ;
      for (int i=1 ; i<N ; i++)
      {
        X[i]=X[i-1] ;
        for (int dd=d-1 ; dd>=0 ; dd--)
        {
          X[i][dd] += 2*m+2*delta ;
          if (X[i][dd]>Boundaries[dd][1]-m-delta)
            X[i][dd] = Boundaries[dd][0]+m+delta ;
          else
            break ;
        }
        if (X[i][0]==Boundaries[0][0]+m+delta) Frozen[i]=true ;
        // randomize the previous grain
        for (int dd=0 ; dd<d ; dd++)
          X[i-1][dd] += (rand()-0.5)*2*delta ;
      }
    }
    else if (!strcmp(line, "roughinclineplane2"))
    {
      printf("Location::roughinclineplane assumes a plane of normal [1,0,0...] at location 0 along the 1st dimension.") ; fflush(stdout) ;
      auto m = *(std::max_element(r.begin(), r.end())) ; // Max radius
      double delta=0.1*m ; int ddd ;
      for (int dd=0 ; dd<d ; dd++) X[0][dd]=Boundaries[dd][0]+m ;
      Frozen[0]=true ; bool bottomlayer=true ;
      for (int i=1 ; i<N ; i++)
      {
        X[i]=X[i-1] ;
        for (ddd=d-1 ; ddd>=0 ; ddd--)
        {
          if (bottomlayer)
          {
            X[i][ddd]+= 2*m ;
            if (X[i][ddd]>Boundaries[ddd][1]-m)
              X[i][ddd] = Boundaries[ddd][0]+m ;
            else
              break ;
          }
          else
          {
            X[i][ddd] += 2*m+2*delta ;
            if (X[i][ddd]>Boundaries[ddd][1]-m-delta)
              X[i][ddd] = Boundaries[ddd][0]+m+delta ;
            else
              break ;
          }
        }
        if (ddd==0) bottomlayer=false ;
        if (bottomlayer)
        {
          Frozen[i-1]=true ;
          X[i-1][0] += rand()*delta ; // Randomness only on x0, and only >r (above the bottom wall)
        }
        else
        {
          for (int dd=0 ; dd<d ; dd++)
            X[i-1][dd] += (rand()-0.5)*2*delta ;
        }
      }
    }
    else if (!strcmp(line, "quasicrystal"))
    {
        auto m = *(std::max_element(r.begin(), r.end())) ; // largest radius
        printf("%g\n", m) ;
        int dd ;
        // X is an array of particle locations
        for (dd=0 ; dd<d ; dd++) { X[0][dd]=Boundaries[dd][0]+m ; } // d is number of dimensions, dd is its index
        for (int i=1 ; i<N ; i++) // number of particles
        {
            X[i]=X[i-1] ; // get previous particle location
            for (dd=0 ; dd<d ; dd++) // iterate over dimensions
            {
                X[i][dd] += 2*m ; // add a diameter
                if (X[i][dd]>Boundaries[dd][1]-m) // if too close to 'right'
                    X[i][dd] = Boundaries[dd][0]+m ; // bring back to 'left'
                else
                    break ;
            }
            if (dd==d) {printf("WARN: cannot affect all particles on the square lattice, not enough space in the simulation box\n") ; break ; }
        }
    }
    else
        printf("ERR: undefined initial location function.\n") ;
}
//-----------------------------
template <int d>
void Parameters<d>::init_radii (char line[], v1d & r)
{
  std::istringstream s(line) ;
  std::string word ;
  double minr, maxr, fraction ;
  boost::random::mt19937 rng(seed);
  boost::random::uniform_01<boost::mt19937> rand(rng) ;

  s >> word ;
  printf("[%s]\n", word.c_str()) ;
  if (word == "uniform")
  {
    s >> minr ; s >> maxr ;
    for (auto & v : r) v = rand()*(maxr-minr)+minr ;
  }
  else if (word == "bidisperse")
  {
    s >> minr ; s >> maxr ;
    s >> fraction ;
    for (auto & v : r) v = (rand()<fraction?minr:maxr) ;
  }
  else
    printf("WARN: unknown radius distribution automatic creation. Nothing done ...\n") ;
}


//======================================
template <int d>
void Parameters<d>::display_info(int tint, v2d& V, v2d& Omega, v2d& F, v2d& Torque, int nct, int ngst)
{
static bool first=true ;
static double Rmax, mmax ;
if (first)
{
 Rmax=*max_element(r.begin(), r.end()) ;
 mmax=*max_element(m.begin(), m.end()) ;
 //printf("\e[10S\e[9A") ;
}

//printf("\e[s%c\n", letters[tint%4]) ;
//printf("\033%c %c\n", 0x37, letters[tint%4]) ;
if (tint%tinfo==0)
{
  //for (int i=0 ; i<(tint*100)/int(T/dt) ; i++) printf("#") ;
  //printf("%d %d %d ",tint, T, (tint*100)/T) ;
 if (!first) printf("\e[8A") ;
 printf("\e[80G") ;
 printf("\n\e[80G NContacts: %d | Nghosts: %d \n", nct, ngst) ;

 double Vmax=Tools<d>::norm(*max_element(V.begin(), V.end(), [](cv1d &a, cv1d &b) {return (Tools<d>::normsq(a)<Tools<d>::normsq(b)) ;})) ;
 double Omegamax=Tools<d>::skewnorm(*max_element(Omega.begin(), Omega.end(), [](cv1d &a, cv1d &b) {return (Tools<d>::skewnormsq(a)<Tools<d>::skewnormsq(b)) ;})) ;
 double Torquemax=Tools<d>::skewnorm(*max_element(Torque.begin(), Torque.end(), [](cv1d &a, cv1d &b) {return (Tools<d>::skewnormsq(a)<Tools<d>::skewnormsq(b)) ;})) ;
 double Fmax=Tools<d>::norm(*max_element(F.begin(), F.end(), [](cv1d &a, cv1d &b) {return (Tools<d>::normsq(a)<Tools<d>::normsq(b)) ;})) ;

 printf("\n") ;
 printf("\e[80G Max V: %15g | V dt / R      = %15g \n", Vmax, Vmax*dt/Rmax) ;
 printf("\e[80G Max O: %15g | Omega dt      = %15g \n", Omegamax, Omegamax*dt) ;
 printf("\e[80G Max F: %15g | F dt dt / m r = %15g \n", Fmax, Fmax*dt*dt/(mmax*Rmax)) ;
 printf("\e[80G Max M: %15g | M dt / m R R  = %15g \n", Torquemax, Torquemax*dt/(mmax*Rmax*Rmax)) ;
 printf("\e[80G                        | g dt dt / R   = %15g \n", Tools<d>::norm(g)*dt*dt/Rmax) ;

//printf("\e[u") ;
//printf("\\033%c", 0x38) ;
printf("\e[0G") ;
fflush(stdout) ;
}
if (first) first=false ;
}
//------------------------------------------
template <int d>
void Parameters<d>::xml_header ()
{
   xmlout->openbranch("boundaries", {make_pair("length", to_string(d*2))}) ;
   for (int i=0 ; i<d ; i++) xmlout->fic << Boundaries[i][0] << " " << Boundaries[i][1] << " " ;
   xmlout->closebranch() ;

   xmlout->openbranch("radius", {make_pair("length", to_string(N))}) ;
   for (auto v: r) xmlout->fic << v << " " ;
   xmlout->closebranch() ;
}

//-----------------------------------------
template <int d>
void Parameters<d>::quit_cleanly()
{
  for (auto v : dumps)
    if ((v.first == ExportType::XML) || (v.first == ExportType::XMLbase64))
      xmlout->emergencyclose() ;
}
template <int d>
void Parameters<d>::finalise()
{
  for (auto v : dumps)
    if ((v.first == ExportType::XML) || (v.first == ExportType::XMLbase64))
      xmlout->close() ;
}


//========================================
template <int d>
int Parameters<d>::dumphandling (int ti, double t, v2d &X, v2d &V, v1d &Vmag, v2d &A, v2d &Omega, v1d &OmegaMag, vector<uint32_t> &PBCFlags, v1d & Z, Multiproc<d> & MP)
{
  static bool xmlstarted=false ;

  for (auto v : dumps)
  {
    if (v.first==ExportType::CSV)
    {
      char path[500] ; sprintf(path, "%s/dump-%05d.csv", Directory.c_str(), ti) ;
      Tools<d>::norm(Vmag, V) ; Tools<d>::norm(OmegaMag, Omega) ;
      if ( v.second & ExportData::VELOCITY)
      {
         Tools<d>::savecsv(path, X, V, r, PBCFlags, Vmag, OmegaMag, Z) ; //These are always written for CSV, independent of the dumplist
      }
      else
      {
          Tools<d>::savecsv(path, X, r, PBCFlags, Vmag, OmegaMag, Z) ; //These are always written for CSV, independent of the dumplist
      }
      if (v.second & ExportData::ORIENTATION)
      {
        char path[500] ; sprintf(path, "%s/dumpA-%05d.csv", Directory.c_str(), ti) ;
        Tools<d>::savecsv(path, A) ;
      }
    }


    if (v.first == ExportType::CSVCONTACT)
    {
     char path[500] ; sprintf(path, "%s/dumpcontactforce-%05d.csv", Directory.c_str(), ti) ;
     FILE * out = fopen(path, "w") ;
     if (out == NULL) {printf("[ERR] Cannot open file for contact force writing.\n") ; fflush(stdout) ; return 1 ; }
     savecsvcontact(out, v.second, MP, X) ;
     fclose(out) ;
    }

    if (v.first == ExportType::VTK)
    {
        v2d tmp ;
        char path[500] ; sprintf(path, "%s/dump-%05d.vtk", Directory.c_str(), ti) ;
        vector <TensorInfos> val;
        if (v.second & ExportData::VELOCITY) val.push_back({"Velocity", TensorType::VECTOR, &V}) ;
        if (v.second & ExportData::OMEGA)    val.push_back({"Omega", TensorType::SKEWTENSOR, &Omega}) ;
        if (v.second & ExportData::OMEGAMAG)  {tmp.push_back(OmegaMag) ; val.push_back({"OmegaMag", TensorType::SCALAR, &tmp}) ;}
        if (v.second & ExportData::ORIENTATION) val.push_back({"ORIENTATION", TensorType::TENSOR, &A}) ;
        if (v.second & ExportData::RADIUS) {tmp.push_back(r) ; val.push_back({"RADIUS", TensorType::SCALAR, &tmp}) ;}
        if (v.second & ExportData::COORDINATION) {tmp.push_back(Z) ; val.push_back({"Coordination", TensorType::SCALAR, &tmp}) ;  }
        if (v.second & ExportData::IDS) {
            vector<double> tmpid (N, 0) ;
            for (int i=0 ; i<N ; i++)
                tmpid[i]=i ;
            tmp.push_back(tmpid) ; val.push_back({"Id", TensorType::SCALAR, &tmp}) ;
        }
        Tools<d>::savevtk(path, N, Boundaries, X, r, val) ;
    }

    if (v.first == ExportType::NETCDFF)
          printf("WARN: netcdf writing haven't been tested and therefore is not plugged in\n") ;

    if (v.first == ExportType::XML)
    {
      if (xmlstarted==false)
      {
        char path[500] ; sprintf(path, "%s/dump.xml", Directory.c_str()) ;
        xmlout->header(d, path) ;
        xml_header() ;
        xmlstarted=true ;
      }
      xmlout->startTS(t);
      if (v.second & ExportData::POSITION) xmlout->writeArray("Position", &X, ArrayType::particles, EncodingType::ascii);
      if (v.second & ExportData::VELOCITY) xmlout->writeArray("Velocity", &V, ArrayType::particles, EncodingType::ascii);
      if (v.second & ExportData::OMEGA)    xmlout->writeArray("Omega", &Omega, ArrayType::particles, EncodingType::ascii);
      if (v.second & ExportData::OMEGAMAG)
          printf("Omega Mag not implemented yet\n");
      if (v.second & ExportData::ORIENTATION) xmlout->writeArray("Orientation", &A, ArrayType::particles, EncodingType::ascii);
      if (v.second & ExportData::COORDINATION) xmlout->writeArray("Coordination", &Z, ArrayType::particles, EncodingType::ascii);
      
      if (v.second & ExportData::IDS) 
      { 
        auto tmp = MP.contacts2array(ExportData::IDS, X, Boundaries) ; 
        xmlout->writeArray("ContactIds", &tmp, ArrayType::contacts, EncodingType::ascii) ;
      }
//         case ExportData::POSITION: res.push_back(loc) ; break ; 
//         case ExportData::FN: res.push_back(contact.infos->Fn) ; break ;
//         case ExportData::FT: res.push_back(contact.infos->Ft) ; break ;
//         case ExportData::GHOSTMASK: res.push_back(contact.infos->ghost) ; break ;
//         case ExportData::GHOSTDIR : res.push_back(contact.infos->ghostdir) ; break ;
//         case ExportData::BRANCHVECTOR: res.push_back(branch) ; break ;
      xmlout->stopTS();
    }

    if (v.first == ExportType::XMLbase64)
    {
      if (xmlstarted==false)
      {
        char path[500] ; sprintf(path, "%s/dump.xml", Directory.c_str()) ;
        xmlout->header(d, path) ;
        xml_header() ;
        xmlstarted=true ;
      }
      xmlout->startTS(t);
      if (v.second & ExportData::POSITION) xmlout->writeArray("Position", &X, ArrayType::particles, EncodingType::base64);
      if (v.second & ExportData::VELOCITY) xmlout->writeArray("Velocity", &V, ArrayType::particles, EncodingType::base64);
      if (v.second & ExportData::OMEGA)    xmlout->writeArray("Omega", &Omega, ArrayType::particles, EncodingType::base64);
      if (v.second & ExportData::OMEGAMAG)
          printf("Omega Mag not implemented yet\n");
      if (v.second & ExportData::ORIENTATION) xmlout->writeArray("Orientation", &A, ArrayType::particles, EncodingType::base64);
      if (v.second & ExportData::COORDINATION) xmlout->writeArray("Orientation", &Z, ArrayType::particles, EncodingType::base64);
      xmlout->stopTS();
    }

  }
return 0 ;
}

#endif
