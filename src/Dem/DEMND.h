#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <vector>
#include <ctime>
#include <cstring>

#include <random>
#ifndef NO_OPENMP
    #include <omp.h>
#endif

#include "cereal/types/vector.hpp"
#include "cereal/types/chrono.hpp"
#include "cereal/types/optional.hpp"
#include "cereal/types/utility.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/map.hpp"
#include "cereal/types/list.hpp"
#include "cereal/archives/binary.hpp"

//#include "callgrind.h"
//#include <ittnotify.h>

#include "Typedefs.h"
#include "Parameters.h"
#include "Contacts.h"
#include "ContactList.h"
#include "Multiproc.h"
#include "Cells.h"
#include "Octree.h"
#include "Boundaries.h"
#ifdef EMSCRIPTEN
    #include <emscripten.h>
    #include <emscripten/bind.h>

    #ifndef JS_CONVERT_ARRAYS
    #define JS_CONVERT_ARRAYS
    using namespace emscripten;
    using Vector2Djs = emscripten::val ;
    using Vector1Djs = emscripten::val ;
 
    template <typename T>
    emscripten::val to_js_array(const std::vector<std::vector<T>>& data) {
        using namespace emscripten;
        val outer = val::array();
        for (size_t i = 0; i < data.size(); ++i) {
            val inner = val::array();
            for (size_t j = 0; j < data[i].size(); ++j) {
                inner.set(j, data[i][j]);
            }
            outer.set(i, inner);
        }
        return outer;
    }
    template <typename T>
    emscripten::val to_js_array(const std::vector<T>& data) {
        using namespace emscripten;
        val outer = val::array();
        for (size_t i = 0; i < data.size(); ++i) {
            outer.set(i, data[i]);
        }
        return outer;
    }
    
    std::vector<double> from_js_array(emscripten::val jsArray) {
    std::vector<double> vec;
    size_t length = jsArray["length"].as<unsigned>();
    vec.reserve(length);
    for (unsigned i = 0; i < length; ++i) {
        vec.push_back(jsArray[i].as<double>());
    }
    return vec;
}   
    #endif
#else
    using Vector2Djs = std::vector<std::vector<double>> ; 
    using Vector1Djs = std::vector<double> ; 
    template <typename T> std::vector<std::vector<T>> to_js_array(std::vector<std::vector<T>>& data) {return data ; }
    template <typename T> std::vector<T> to_js_array(std::vector<T>& data) {return data ; }
    template <typename T> std::vector<T> from_js_array(std::vector<T>& data) {return data ; }
#endif
    
/** \weakgroup API
 * These functions are useful for external access, for interactive runs. This is the basic flow of API calls to run an interactive simulation:
 * \dot
digraph C {
rankdir="LR";
Input -> interpret_command ;
Input [label="Simulation<d>"] ;
interpret_command -> interpret_command [dir=back] ;
interpret_command -> finalise_init ;
finalise_init -> step_forward ;
step_forward -> step_forward [dir=back] ;
step_forward -> setX ;
setX -> step_forward ;
step_forward -> externalforce ;
externalforce -> step_forward ;
step_forward -> finalise ;
{rank=same ; step_forward ; setX ; externalforce ;}
}
\enddot
 *
 */

/** \addtogroup DEM Discrete Element Simulations
 * This module handles the Discrete Element Simulations.
 *  @{ */

extern vector <std::pair<ExportType,ExportData>> * toclean ;
extern XMLWriter * xmlout ;


template <int d>
class Simulation {
public:
    Parameters<d> P ;
    int N ;
    std::vector < std::vector <double> > X ;
    std::vector < std::vector <double> > V ;
    std::vector < std::vector <double> > A ;
    std::vector < std::vector <double> > Omega ;
    std::vector < std::vector <double> > F ;
    std::vector < std::vector <double> > FOld ;
    std::vector < std::vector <double> > Torque ;
    std::vector < std::vector <double> > TorqueOld ;
    std::vector < double > Vmag ;
    std::vector < double > OmegaMag ;
    std::vector < double > Z ;
    std::vector < std::vector <double> > Fcorr ;
    std::vector < std::vector <double> > TorqueCorr  ;
        
    std::vector < std::optional<int> > RigidBodyId ; 

    std::vector <SpecificAction<d>> ExternalAction ;

    std::vector <uint32_t> PBCFlags ;
    std::vector < std::vector <double> > WallForce ;
    std::vector<std::vector<double>> empty_array ;
    std::vector < std::vector <double> > ParticleForce ;

    vector <uint32_t> Ghost ;
    vector <uint32_t> Ghost_dir ;
    v1d Atmp ;

    int numthread=1 ;
    Multiproc<d> MP ;
    Cells<d> cells ; 
    Octree<d> octree ; 

    double t ; int ti ;
    double dt ;
    clock_t tnow, tprevious ;

    //-----------------------------------------
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar(N, 
           X, V, A, Omega, F, FOld, Torque, TorqueOld, Vmag, OmegaMag, Z, Fcorr, TorqueCorr, RigidBodyId,
           PBCFlags, WallForce, empty_array, ParticleForce, Ghost, Ghost_dir, Atmp, 
           numthread, t, ti, dt, tnow, tprevious
           , P
           // Todo: ExternalAction, MP, cells, 
        );
        printf("RESTART ERROR: STILL SOME THINGS TO SERIALIZE IN Simulation") ; 
    }
    //-----------------------------------------
    
    //==========================================================
    Simulation(int NN) {
        P = Parameters<d>(NN) ;
        Tools<d>::initialise() ;
        if (!Tools<d>::check_initialised(d)) printf("ERR: Something terribly wrong happened\n") ;
        assert(d<(static_cast<int>(sizeof(int))*8-1)) ; //TODO
        // Array initialisations
        N=P.N ;
        X.resize(N, std::vector <double> (d, 0)) ;
        // X[0][0] = 123;
        V.resize(N, std::vector <double> (d, 0)) ;
        A.resize(N, std::vector <double> (d*d, 0)) ; for (int i=0 ; i<N ; i++) A[i]=Tools<d>::Eye ;
        Omega.resize(N, std::vector <double> (d*(d-1)/2, 0)) ; //Rotational velocity matrix (antisymetrical)
        F.resize(N, std::vector <double> (d, 0)) ;
        FOld.resize(N, std::vector <double> (d, 0)) ;
        Torque.resize(N, std::vector <double> (d*(d-1)/2, 0)) ;
        TorqueOld.resize(N, std::vector <double> (d*(d-1)/2, 0)) ;

        Vmag.resize (N,0) ;
        OmegaMag.resize (N,0) ;
        Z.resize (N,0) ;
        Fcorr.resize (N, std::vector <double> (d, 0)) ;
        TorqueCorr.resize (N, std::vector <double> (d*(d-1)/2, 0)) ;
        RigidBodyId.resize(N,{}) ; 

        PBCFlags.resize (N, 0) ;
        WallForce.resize (2*d, std::vector <double> (d,0)) ;
        empty_array.resize(1, std::vector <double> (1, 0)) ;

        Ghost.resize (N, 0) ;
        Ghost_dir.resize (N, 0) ;

        Atmp.resize (d*d, 0) ;

        // Initial state setup
        P.set_boundaries() ;
    }
    //-------------------------------------------------------------------------
    /** \brief Initialise the simulation from a file
    */
    void init_from_file (char filename[])
    {
      P.load_datafile (filename, X, V, Omega) ;
    }
    //-------------------------------------------------------------------------
    /** \brief Tell NDDEM that the simulations are now initialised and we can start running.
     * \warning It is important to remember to call this function in interactive run!
     * \ingroup API
    */
    void finalise_init () {
        //if (strcmp(argv[3], "default"))
        //    P.load_datafile (argv[3], X, V, Omega) ;
        toclean = &(P.dumps) ;
        xmlout = P.xmlout ;

        P.RigidBodies.allocate (RigidBodyId) ; 

        #ifndef NO_OPENMP
          const char* env_p = std::getenv("OMP_NUM_THREADS") ;
          if (env_p!=nullptr) numthread = atoi (env_p) ;
          omp_set_num_threads(numthread) ;
        #endif

        MP.initialise(N, numthread, P) ;
        
        if (P.contact_strategy == ContactStrategies::CELLS) 
        {
            printf("INITIALISE CELLS\n") ; fflush(stdout) ;
            auto rmax = std::max_element(P.r.begin(), P.r.end()) ;
            printf("%g ", *rmax) ;
            cells.init_cells(P.Boundaries, P.cellsize==-1?(*rmax*2.1):P.cellsize) ;
            MP.splitcells(cells.cells.size()) ; 
            MP.CLp_it.init(N, numthread) ; 
        }
        else if (P.contact_strategy == ContactStrategies::OCTREE)
        {
            printf("INITIALISE OCTREE\n") ; fflush(stdout) ;
            octree.init_cells(P.Boundaries, P.r) ;
            MP.splitcells(octree.cells_to_split()) ; 
            MP.CLp_it.init(N, numthread) ; 
        }
        
        dt=P.dt ;
        t=0 ; ti=0 ;
        tprevious=clock() ;
        printf("[INFO] Orientation tracking is %s\n", P.orientationtracking?"True":"False") ;
    }
    //-------------------------------------------------------------------
    /** \brief Interpret individual script command from string
     * \param in Input command string
     * \ingroup API
    */
    void interpret_command (string in)
    {
        P.interpret_command(in, X,V,Omega) ;
    }
    //--------------------------------------------------------------------
    /** \brief Run the whole simulation as defined in the input script
     * \param nt number of timestep to advance
    */
    void step_forward_all ()
    {
      int nsteps = P.T/dt ;
      step_forward(nsteps) ;
    }
    //----
    /** \brief Advance the simulation for nt steps (actual duration nt*dt).
     * \param nt number of timestep to advance
     * \ingroup API
    */
    void step_forward (int nt)
    {        
        
      for (int ntt=0 ; ntt<nt ; ntt++, t+=dt, ti++)
      {
        //if (ntt==990) {CALLGRIND_START_INSTRUMENTATION ;}
          
        // printf("UP TO TIME: %g with timestep: %g\n", t, dt);
        // printf("%g %g %g\n", X[0][0],X[0][1],X[0][2]);
        bool isdumptime = (ntt==nt-1) || (ti % P.tdump==0) ;
        //P.display_info(ti, V, Omega, F, Torque, 0, 0) ;
        if (ti%P.tinfo==0)
        {
            tnow = clock();
            printf("\r%10g | %5.2g%% | %d iterations in %10gs | %5d | finish in %10gs",t, t/P.T*100, P.tinfo,
                    double(tnow - tprevious) / CLOCKS_PER_SEC, ti, ((P.T-t)/(P.tinfo*dt))*(double(tnow - tprevious) / CLOCKS_PER_SEC)) ;
            //fprintf(logfile, "%d %10g %lu %lu\n", ti, double(tnow - tprevious) / CLOCKS_PER_SEC, MP.CLp[0].v.size(), MP.CLw[0].v.size()) ;
            fflush(stdout) ;
            tprevious=tnow ;
        }
        
        //----- Velocity Verlet step 1 : compute the new positions
        #pragma omp parallel for default(none) shared (N) shared(X) shared(P) shared(V) shared(FOld) shared(Omega) shared(PBCFlags) shared(dt) shared(Ghost) shared(Ghost_dir) shared(A)
        for (int i=0 ; i<N ; i++)
        {
            double disp, totdisp=0 ;
            for (int dd=0 ; dd<d ; dd++)
            {
                disp = V[i][dd]*dt + FOld[i][dd] * (dt * dt / P.m[i] /2.) ;
                X[i][dd] += disp  ;
                totdisp += disp*disp ;
            }
            //displacement[i] += sqrt(totdisp) ;
            //if (displacement[i] > maxdisp[0]) {maxdisp[1]=maxdisp[0] ; maxdisp[0]=displacement[i] ; } // ERROR RACE CONDITION ON MAXDISP

            // Simpler version to make A evolve (Euler, doesn't need to be accurate actually, A is never used for the dynamics), and Gram-Shmidt orthonormalising after ...
            if (P.orientationtracking)
            {
                v1d tmpO (d*d,0), tmpterm1 (d*d,0) ;
                Tools<d>::skewexpand(tmpO, Omega[i]) ;
                Tools<d>::matmult(tmpterm1, tmpO, A[i]) ;
                for (int dd=0 ; dd<d*d ; dd++)
                    A[i][dd] -= tmpterm1[dd] * dt ;
                Tools<d>::orthonormalise(A[i]) ;
            }

            // Boundary conditions ...
            P.perform_PBC(X[i], PBCFlags[i]) ;
            P.perform_PBCLE(X[i], V[i], PBCFlags[i]) ;

            // Find ghosts
            Ghost[i]=0 ; Ghost_dir[i]=0 ;
            uint32_t mask=1 ;

            for (size_t j=0 ; j<P.Boundaries.size() ; j++, mask<<=1)
            {
                if (!P.Boundaries[j].is_periodic()) continue ;
                if      (X[i][j] <= P.Boundaries[j].xmin + P.r[i]) {Ghost[i] |= mask ; }
                else if (X[i][j] >= P.Boundaries[j].xmax - P.r[i]) {Ghost[i] |= mask ; Ghost_dir[i] |= mask ;}
            }
            
            if (P.Boundaries[0].Type == WallType::PBC_LE && (Ghost[i]&1)) // We need to consider the case where we have a ghost through the LE_PBC
            {
                mask = (1<<30) ; // WARNING dim 30 will be used for LEPBC!!
                double tmpyloc = X[i][1] + (Ghost_dir[i]&1?-1:1)*P.Boundaries[0].displacement ;
                if (tmpyloc > P.Boundaries[1].xmax) tmpyloc -= P.Boundaries[1].delta ;
                if (tmpyloc < P.Boundaries[1].xmin) tmpyloc += P.Boundaries[1].delta ;
                if      (tmpyloc <= P.Boundaries[1].xmin + P.r[i]) {Ghost[i] |= mask ; }
                else if (tmpyloc >= P.Boundaries[1].xmax - P.r[i]) {Ghost[i] |= mask ; Ghost_dir[i] |= mask ;}

            }

            if (P.forceinsphere)
                P.perform_forceinsphere(X[i]) ;

            //Nghosts=Ghosts.size() ;
        } // END PARALLEL SECTION
        
        P.perform_MOVINGWALL() ;
        P.perform_PBCLE_move() ;
        std::invoke (P.update_gravity, &P, dt) ;
        if (P.contact_strategy == ContactStrategies::CELLS) cells.allocate_to_cells(X) ; 
        else if (P.contact_strategy == ContactStrategies::OCTREE) octree.allocate_to_cells(X) ; 

        // ----- Contact detection ------
        double LE_displacement ; 
        if (P.Boundaries[0].Type == WallType::PBC_LE) 
            LE_displacement = P.Boundaries[0].displacement ; 
        else
            LE_displacement = 0 ; 
          
        for (int i=0 ; i<MP.P ; i++)
            MP.CLp_it.it_ends[i] = MP.CLp[i].v.end() ;
          
        #pragma omp parallel default(none) shared(MP) shared(P) shared(N) shared(X) shared(Ghost) shared(Ghost_dir) shared(RigidBodyId) shared (stdout) shared(cells) shared(LE_displacement)
        {
         #ifdef NO_OPENMP
         int ID = 0 ;
         #else
         int ID = omp_get_thread_num();
         double timebeg = omp_get_wtime();
         #endif
         cpm<d> tmpcpm(0,0,0,0,nullptr) ; 
         cp<d> tmpcp(0,0,0,nullptr) ; double sum=0 ;
         
         ContactList<d> & CLw = MP.CLw[ID] ; ContactListMesh<d> & CLm = MP.CLm[ID] ; 
         CLw.reset(); CLm.reset() ; 
        
         if (P.contact_strategy == ContactStrategies::CELLS)
         {
             cells.contacts(ID, {MP.sharecell[ID], MP.sharecell[ID+1]}, MP.CLp_it, MP.CLp_new[ID], X, P.r, LE_displacement) ; 
         }
         else if (P.contact_strategy == ContactStrategies::OCTREE)
         {
             octree.contacts(ID, {MP.sharecell[ID], MP.sharecell[ID+1]}, MP.CLp_it, MP.CLp_new[ID], X, P.r, LE_displacement) ; 
         }
         else
         {
            ContactList<d> & CLp = MP.CLp[ID] ;
            CLp.reset() ; 
            
            for (int i=MP.share[ID] ; i<MP.share[ID+1] ; i++)
            {
                // Contact detection between particles
                tmpcp.setinfo(CLp.default_action());
                tmpcp.i=i ;
                for (int j=i+1 ; j<N ; j++) // Regular particles
                {
                    
                    if (RigidBodyId[i].has_value() && RigidBodyId[j].has_value() &&  RigidBodyId[i]==RigidBodyId[j]) continue ; 
                    if (Ghost[j] | Ghost[i])
                    {
                        tmpcp.j=j ; tmpcp.ghostdir=Ghost_dir[j] | (Ghost[i]&(~Ghost_dir[i])) ;
                        bitdim tmpghost = Ghost[j] | Ghost[i] ;
                        if (P.Boundaries[0].Type == WallType::PBC_LE && (Ghost[i]&1))
                        {                            
                            double tmpyloc = X[j][1] + (tmpcp.ghostdir&1?-1:1)*P.Boundaries[0].displacement ;
                            if (tmpyloc > P.Boundaries[1].xmax) tmpyloc -= P.Boundaries[1].delta ;
                            if (tmpyloc < P.Boundaries[1].xmin) tmpyloc += P.Boundaries[1].delta ;
                            if      (tmpyloc <= P.Boundaries[1].xmin + P.r[i]) {tmpghost |= (1<<30)  ; }
                            else if (tmpyloc >= P.Boundaries[1].xmax - P.r[i]) {tmpghost |= (1<<30)  ; tmpcp.ghostdir |= (1<<30)  ;}
                        }
                        
                        //CLp.check_ghost (Ghost[j], P, X[i], X[j], tmpcp) ;
                        (CLp.*CLp.check_ghost) (tmpghost, P, X[i], X[j], P.r[i], P.r[j], tmpcp, 0,0,0) ;
                    }
                    else
                    {
                        sum=0 ;
                        for (int k=0 ; sum<(P.r[i]+P.r[j])*(P.r[i]+P.r[j]) && k<d ; k++) sum+= (X[i][k]-X[j][k])*(X[i][k]-X[j][k]) ;
                        if (sum<(P.r[i]+P.r[j])*(P.r[i]+P.r[j]))
                        {
                            tmpcp.j=j ; tmpcp.contactlength=sqrt(sum) ; tmpcp.ghost=0 ; tmpcp.ghostdir=0 ;
                            CLp.insert(tmpcp) ;
                        }
                    }
                }
            }
            CLp.finalise() ;
         }
         
         for (int i=MP.share[ID] ; i<MP.share[ID+1] ; i++)
         {
            // Contact detection between particles and walls
            tmpcp.setinfo(CLw.default_action());
            tmpcp.i=i ; tmpcp.ghost=0 ;
            for (size_t j=0 ; j<P.Boundaries.size() ; j++) // Wall contacts
            {
                    if (P.Boundaries[j].Type==WallType::PBC) continue ;

                    if (P.Boundaries[j].Type==WallType::WALL || P.Boundaries[j].Type==WallType::MOVINGWALL)
                    {
                        tmpcp.contactlength=fabs(X[i][j]-P.Boundaries[j].xmin) ;
                        if (tmpcp.contactlength<P.r[i])
                        {
                            tmpcp.j=(2*j+0);
                            CLw.insert(tmpcp) ;
                        }

                        tmpcp.contactlength=fabs(X[i][j]-P.Boundaries[j].xmax) ;
                        if (tmpcp.contactlength<P.r[i])
                        {
                            tmpcp.j=(2*j+1);
                            CLw.insert(tmpcp) ;
                        }
                    }
                    else if (P.Boundaries[j].Type==WallType::SPHERE || 
                             P.Boundaries[j].Type==WallType::HEMISPHERE || 
                             P.Boundaries[j].Type==WallType::ROTATINGSPHERE)
                    {
                        tmpcp.contactlength=0 ;
                        for (int dd=0 ; dd<d ; dd++)
                            tmpcp.contactlength += (P.Boundaries[j].center[dd] - X[i][dd])*(P.Boundaries[j].center[dd] - X[i][dd]) ;
                        
                        if (tmpcp.contactlength < (P.Boundaries[j].radius + P.r[i])*(P.Boundaries[j].radius + P.r[i]) &&
                            tmpcp.contactlength > (P.Boundaries[j].radius - P.r[i])*(P.Boundaries[j].radius - P.r[i]))
                        {
                            tmpcp.contactlength = sqrt(tmpcp.contactlength) - P.Boundaries[j].radius ;
                            if (tmpcp.contactlength < 0) tmpcp.j=2*j;
                            else tmpcp.j=2*j+1 ;
                            tmpcp.contactlength = fabs(tmpcp.contactlength) ;
                            CLw.insert(tmpcp) ;
                        }
                    }
                    else if (P.Boundaries[j].Type==WallType::AXIALCYLINDER)
                    {                        
                        tmpcp.contactlength=0 ;
                        
                        for (int dd=0 ; dd<d ; dd++)
                        {
                            if (dd == P.Boundaries[j].axis) continue ; 
                            tmpcp.contactlength += (P.Boundaries[j].center[dd] - X[i][dd])*(P.Boundaries[j].center[dd] - X[i][dd]) ;
                        }
                        if (tmpcp.contactlength < (P.Boundaries[j].radius + P.r[i])*(P.Boundaries[j].radius + P.r[i]) &&
                            tmpcp.contactlength > (P.Boundaries[j].radius - P.r[i])*(P.Boundaries[j].radius - P.r[i]))
                        {
                            tmpcp.contactlength = sqrt(tmpcp.contactlength) - P.Boundaries[j].radius ;
                            if (tmpcp.contactlength < 0) tmpcp.j=2*j;
                            else tmpcp.j=2*j+1 ;
                            tmpcp.contactlength = fabs(tmpcp.contactlength) ;
                            CLw.insert(tmpcp) ;
                        }
                    }
                    else if (P.Boundaries[j].Type==WallType::ELLIPSE)
                    {
                        static_assert((sizeof(double)==8)) ;
                        
                        double tparam ; 
                        std::tie(tparam, tmpcp.contactlength) = Tools_2D::contact_ellipse_disk (X[i], P.Boundaries[j].semiaxisx, P.Boundaries[j].semiaxisy, P.Boundaries[j].centerx, P.Boundaries[j].centery, P.graddesc_gamma, P.graddesc_tol) ; 
                        if (tmpcp.contactlength<P.r[i])
                        {
                            tmpcp.j=2*j ;
                            #pragma GCC diagnostic push
                            #pragma GCC diagnostic ignored "-Wstrict-aliasing"
                            tmpcp.ghost = *(reinterpret_cast<uint32_t*>(&tparam)) ;
                            tmpcp.ghostdir=*(reinterpret_cast<uint64_t*>(&tparam))>>32 ;      //ugly bit punning, I'm so sorry...
                            #pragma GCC diagnostic pop
                            CLw.insert(tmpcp) ;
                        }                       
                    }
            }
            
            // Contact detection between particles and meshes
            tmpcpm.setinfo(CLm.default_action()) ; 
            tmpcpm.i=i ;
            for (size_t j=0 ; j<P.Meshes.size() ; j++) //TODO Meshes incompatible with PBC
            {
                bool a= CLm.check_mesh_dst_contact(P.Meshes[j], X[i], P.r[i], tmpcpm) ;
                //printf("%d %g %g |", a, tmpcpm.contactlength, P.skin) ; 
                if (a && tmpcpm.contactlength < P.r[i]) 
                {
                    CLm.insert(tmpcpm) ; 
                }
            }
        }
        CLw.finalise() ;
        CLm.finalise() ;
        #ifndef NO_OPENMP
        MP.timing_contacts[ID] += omp_get_wtime()-timebeg;
        #endif
        } //END PARALLEL SECTION
      
        if (P.contact_strategy == ContactStrategies::CELLS || P.contact_strategy == ContactStrategies::OCTREE)
        {
            MP.merge_newcontacts() ; 
            for (int i=0 ; i<N ; i++)
                MP.CLp_it.it_array_beg[i] = MP.CLp_it.null_list.v.begin() ; 
        }
          
        //-------------------------------------------------------------------------------
        // Force and torque computation
        Tools<d>::setzero(F) ; Tools<d>::setzero(Fcorr) ; Tools<d>::setzero(TorqueCorr) ;
        Tools<d>::setgravity(F, P.g, P.m); // Actually set gravity is effectively also doing the setzero
        Tools<d>::setzero(Torque);

        for (auto it = ExternalAction.begin() ; it<ExternalAction.end() ; /*KEEP EMPTY*/ )
        {
            Tools<d>::vAddFew(F[it->id], it->Fn, it->Ft, Fcorr[it->id]) ;
            Tools<d>::vAddOne(Torque[it->id], it->Torquei, TorqueCorr[it->id]) ;

            it->duration -- ;
            if (it->duration<=0)
                ExternalAction.erase(it) ;
            else
                it++ ;
        }
        
        //Particle - particle contacts
        #pragma omp parallel default(none) shared(MP) shared(P) shared(X) shared(V) shared(Omega) shared(F) shared(Fcorr) shared(TorqueCorr) shared(Torque) shared(stdout) shared(isdumptime)
        {
          #ifdef NO_OPENMP
            int ID = 0 ;
          #else
            int ID = omp_get_thread_num();
            double timebeg = omp_get_wtime();
          #endif
            ContactList<d> & CLp = MP.CLp[ID] ; ContactList<d> & CLw = MP.CLw[ID] ; ContactListMesh<d> & CLm = MP.CLm[ID] ; 
            Contacts<d> & C =MP.C[ID] ;
            v1d tmpcn (d,0) ; v1d tmpvel (d,0) ;

            // Particle-particle contacts
            //printf("====%d %g=====\n", ti, P.Boundaries[0].displacement) ;
            int curi = -1 ; 
            for (auto it = CLp.v.begin() ; it!=CLp.v.end() ; it++)
            {
                // We need to do some contact handling first
                while (it != CLp.v.end() && !it->persisting) 
                    it = CLp.v.erase(it) ; 
                if (it == CLp.v.end())
                {
                    /*if (curi!= -1) 
                        MP.CLp_it.it_array_end[curi] = CLp.v.end() ; */
                    break ; 
                }
                
                if ( it->i != curi )
                {
                    MP.CLp_it.it_array_beg[it->i] = it ; 
                    /*if (curi != -1) 
                        MP.CLp_it.it_array_end[curi] = it ; */
                    curi = it->i ;
                }    
                
                // Proceed if the contact needs to be considered                
                if (it->ghost==0)
                {
                    C.particle_particle(X[it->i], V[it->i], Omega[it->i], P.r[it->i], P.m[it->i],
                                        X[it->j], V[it->j], Omega[it->j], P.r[it->j], P.m[it->j], *it, isdumptime) ;
                }
                else
                {
                    // printf("%d\n", it->ghost) ; fflush(stdout) ; 
                    (C.*C.particle_ghost) (X[it->i], V[it->i], Omega[it->i], P.r[it->i], P.m[it->i],
                                        X[it->j], V[it->j], Omega[it->j], P.r[it->j], P.m[it->j], *it, isdumptime);//, logghosts) ;
                }
                if (isdumptime) it->saveinfo(C.Act) ;

                Tools<d>::vAddFew(F[it->i], C.Act.Fn, C.Act.Ft, Fcorr[it->i]) ;
                Tools<d>::vAddOne(Torque[it->i], C.Act.Torquei, TorqueCorr[it->i]) ;

                if (MP.ismine(ID,it->j))
                {
                    Tools<d>::vSubFew(F[it->j], C.Act.Fn, C.Act.Ft, Fcorr[it->j]) ;
                    Tools<d>::vAddOne(Torque[it->j], C.Act.Torquej, TorqueCorr[it->j]) ;
                }
                else
                    MP.delaying(ID, it->j, C.Act) ;
                
                it->persisting = false ; 

                //Tools<d>::vAdd(F[it->i], Act.Fn, Act.Ft) ; Tools<d>::vSub(F[it->j], Act.Fn, Act.Ft) ; //F[it->i] += (Act.Fn + Act.Ft) ; F[it->j] -= (Act.Fn + Act.Ft) ;
                //Torque[it->i] += Act.Torquei ; Torque[it->j] += Act.Torquej ;
            }
            //if (curi!= -1) 
            //    MP.CLp_it.it_array_end[curi] = CLp.v.end() ; 
            
            // Particle wall contacts
            for (auto it = CLw.v.begin() ; it!=CLw.v.end() ; it++)
            {
                if (P.Boundaries[it->j/2].Type == WallType::SPHERE ||
                    P.Boundaries[it->j/2].Type == WallType::HEMISPHERE)
                {
                    if ( P.Boundaries[it->j/2].Type == WallType::HEMISPHERE && 
                        X[it->i][P.Boundaries[it->j/2].axis]>P.Boundaries[it->j/2].center[P.Boundaries[it->j/2].axis]+P.r[it->i]) continue ; 
                    for (int dd = 0 ; dd<d ; dd++)
                        tmpcn[dd] = (X[it->i][dd]-P.Boundaries[it->j/2].center[dd])*((it->j%2==0)?-1:1) ;
                    tmpcn/=Tools<d>::norm(tmpcn) ;
                    C.particle_wall( V[it->i],Omega[it->i],P.r[it->i], P.m[it->i], tmpcn, *it) ;
                }
                else if (P.Boundaries[it->j/2].Type == WallType::AXIALCYLINDER)
                {
                    tmpcn[P.Boundaries[it->j/2].axis]= 0 ; 
                    for (int dd = 0 ; dd<d ; dd++)
                    {
                        if (dd == P.Boundaries[it->j/2].axis) continue ; 
                        tmpcn[dd] = (X[it->i][dd]-P.Boundaries[it->j/2].center[dd])*((it->j%2==0)?-1:1) ;
                    }
                    tmpcn/=Tools<d>::norm(tmpcn) ;
                    C.particle_wall( V[it->i],Omega[it->i],P.r[it->i], P.m[it->i], tmpcn, *it) ;                    
                }
                else if (P.Boundaries[it->j/2].Type == WallType::ROTATINGSPHERE)
                {
                    for (int dd = 0 ; dd<d ; dd++)
                        tmpcn[dd] = (X[it->i][dd]-P.Boundaries[it->j/2].center[dd])*((it->j%2==0)?-1:1) ;
                    tmpcn/=Tools<d>::norm(tmpcn) ;
                    Tools<d>::surfacevelocity(tmpvel, X[it->i]+tmpcn*(-P.r[it->i]), &(P.Boundaries[it->j/2].center[0]) , nullptr, &(P.Boundaries[it->j/2].omega[0])) ;
                    //printf("%g | %g %g | %g %g | %g %g\n", P.Boundaries[it->j/2][4+2], (X[it->i]+tmpcn*(-P.r[it->i]))[0], (X[it->i]+tmpcn*(-P.r[it->i]))[1], P.Boundaries[it->j/2][4], P.Boundaries[it->j/2][5], tmpvel[0], tmpvel[1]) ; fflush(stdout) ;
                    C.particle_movingwall(V[it->i],Omega[it->i],P.r[it->i], P.m[it->i], tmpcn, tmpvel, *it) ;
                }
                else if (P.Boundaries[it->j/2].Type == WallType::ELLIPSE)
                {
                    assert((d==2)) ; 
                    std::vector<double> tmpcn(2) ; 
                    #pragma GCC diagnostic push
                    #pragma GCC diagnostic ignored "-Wstrict-aliasing"
                    uint64_t c=((uint64_t)(it->ghostdir) <<32 | (uint64_t)(it->ghost)) ;
                    double tparam = *reinterpret_cast<double*>(&c) ; 
                    #pragma GCC diagnostic pop
                    tmpcn[0]=X[it->i][0]-(P.Boundaries[it->j/2].centerx+P.Boundaries[it->j/2].semiaxisx*cos(tparam)) ; 
                    tmpcn[1]=X[it->i][1]-(P.Boundaries[it->j/2].centery+P.Boundaries[it->j/2].semiaxisy*sin(tparam)) ; 
                    tmpcn/=sqrt(tmpcn[0]*tmpcn[0]+tmpcn[1]*tmpcn[1]);
                    C.particle_wall( V[it->i],Omega[it->i],P.r[it->i], P.m[it->i], tmpcn, *it) ;
                }
                else
                {

                    Tools<d>::unitvec(tmpcn, it->j/2) ;
                    tmpcn=tmpcn*(-((it->j%2==0)?-1:1)) ; // l give the orientation (+1 or -1)
                    C.particle_wall( V[it->i],Omega[it->i],P.r[it->i], P.m[it->i], tmpcn, *it) ;
                }

                //Tools<d>::vAdd(F[it->i], Act.Fn, Act.Ft) ; // F[it->i] += (Act.Fn+Act.Ft) ;
                //Torque[it->i] += Act.Torquei ;

                Tools<d>::vAddFew(F[it->i], C.Act.Fn, C.Act.Ft, Fcorr[it->i]) ;
                Tools<d>::vAddOne(Torque[it->i], C.Act.Torquei, TorqueCorr[it->i]) ;

                if ( P.wallforcecompute || ( P.wallforcerequested && !P.wallforcecomputed ) ) MP.delayingwall(ID, it->j, C.Act) ;
            }
            
            // Particle mesh contacts
            for (auto it = CLm.v.begin() ; it != CLm.v.end() ; it++)
            {
                //printf("@ %g | %g %g %g | %d \n", it->contactlength, it->contactpoint[0], it->contactpoint[1], it->contactpoint[2], it->submeshid) ; fflush(stdout) ; 
                C.particle_mesh ( X[it->i], V[it->i], Omega[it->i], P.r[it->i], P.m[it->i], *it) ;
                
                Tools<d>::vAddFew(F[it->i], C.Act.Fn, C.Act.Ft, Fcorr[it->i]) ;
                Tools<d>::vAddOne(Torque[it->i], C.Act.Torquei, TorqueCorr[it->i]) ;
            }
            
            #ifndef NO_OPENMP
            MP.timing_forces[ID] += omp_get_wtime()-timebeg;
            #endif
        } //END PARALLEL PART
        //ParticleForce = calculateParticleForce() ;

        // Finish by sequencially adding the grains that were not owned by the parallel proc when computed
        for (int i=0 ; i<MP.P ; i++)
        {
            for (uint j=0 ; j<MP.delayed_size[i] ; j++)
            {
            Tools<d>::vSubFew(F[MP.delayedj[i][j]], MP.delayed[i][j].Fn, MP.delayed[i][j].Ft, Fcorr[MP.delayedj[i][j]]) ;
            Tools<d>::vAddOne(Torque[MP.delayedj[i][j]], MP.delayed[i][j].Torquej, TorqueCorr[MP.delayedj[i][j]]) ;
            }
        }
        
        MP.delayed_clean() ;

        // Benchmark::stop_clock("Forces");
        //---------- Velocity Verlet step 3 : compute the new velocities
        // Benchmark::start_clock("Verlet last");
        
        P.RigidBodies.process_forces(V, F, Torque, P.m, P.g) ;
        
        
        #pragma omp parallel for default(none) shared(N) shared(P) shared(V) shared(Omega) shared(F) shared(FOld) shared(Torque) shared(TorqueOld) shared(dt)
        for (int i=0 ; i<N ; i++)
        {
            //printf("%10g %10g %10g\n%10g %10g %10g\n%10g %10g %10g\n\n", A[0][0], A[0][1], A[0][2], A[0][3], A[0][4], A[0][5], A[0][6], A[0][7], A[0][8]) ;
            if (P.Frozen[i]) {Tools<d>::setzero(TorqueOld[i]) ; Tools<d>::setzero(F[i]) ; Tools<d>::setzero(FOld[i]) ; /*Tools<d>::setzero(V[i]) ; */ Tools<d>::setzero(Omega[i]) ; }
            
            Tools<d>::vAddScaled(V[i], dt/2./P.m[i], F[i], FOld[i]) ; //V[i] += (F[i] + FOld[i])*(dt/2./P.m[i]) ;
            Tools<d>::vAddScaled(Omega[i], dt/2./P.I[i], Torque[i], TorqueOld[i]) ; // Omega[i] += (Torque[i]+TorqueOld[i])*(dt/2./P.I[i]) ;
            if ( !P.Frozen[i] && P.damping > 0.0 ) { 
                Tools<d>::vSubScaled(Omega[i], P.damping, Omega[i]) ; // BENJY - add damping to Omega
                Tools<d>::vSubScaled(V[i], P.damping, V[i]) ; // BENJY - add damping to velocity
            }
            
            FOld[i]=F[i] ;
            TorqueOld[i]=Torque[i] ;
        } // END OF PARALLEL SECTION

        // Benchmark::stop_clock("Verlet last");
        
        
        // Check events
        P.check_events(t, X,V,Omega) ;

        if ( P.wallforcerequested && !P.wallforcecomputed )
        {
          P.wallforcecomputed = true;
          Tools<d>::setzero(WallForce) ;
          for (int i=0 ; i<MP.P ; i++)
            for (uint j=0 ; j<MP.delayedwall_size[i] ; j++)
              Tools<d>::vSubFew(WallForce[MP.delayedwallj[i][j]], MP.delayedwall[i][j].Fn, MP.delayedwall[i][j].Ft) ;
        }

        // Output something at some point I guess
        if (ti % P.tdump==0)
        {
            Tools<d>::setzero(Z) ; for (auto &v: MP.CLp) v.coordinance(Z) ;
            P.dumphandling (ti, t, X, V, Vmag, A, Omega, OmegaMag, PBCFlags, Z, MP) ;
            std::fill(PBCFlags.begin(), PBCFlags.end(), 0);

            if (P.wallforcecompute)
            {
              char path[5000] ; sprintf(path, "%s/LogWallForce-%05d.txt", P.Directory.c_str(), ti) ;
              Tools<d>::setzero(WallForce) ;

              for (int i=0 ; i<MP.P ; i++)
                for (uint j=0 ; j<MP.delayedwall_size[i] ; j++)
                  Tools<d>::vSubFew(WallForce[MP.delayedwallj[i][j]], MP.delayedwall[i][j].Fn, MP.delayedwall[i][j].Ft) ;

            Tools<d>::savetxt(path, WallForce, ( char const *)("Force on the various walls")) ;
            }
            
            //fprintf(stderr, "%g %g\n", X[0][0], X[0][1]) ;
        }

        if (P.wallforcecompute || P.wallforcecomputed) MP.delayedwall_clean() ;
        
        
        // Load balancing on the procs as needed
        #ifndef NO_OPENMP
        MP.num_time++ ;
        if (MP.num_time>50)
        {    
          MP.load_balance(P.contact_strategy) ;
          // Cleaning the load balancing
          MP.num_time = 0 ;
          MP.timing_contacts = vector<double>(MP.P,0) ;
          MP.timing_forces = vector<double>(MP.P,0) ;
        }
        #endif
        
        //save_restart(*this, "restart_archive.bin") ; 
    }
  }

  //-------------------------
  /** \brief Settles the simulation, closing open files etc.
    * \ingroup API
    */
  void finalise ()
  {
    P.finalise() ;
    printf("This is the end ...\n") ;
    //fclose(logfile) ;
  }

  //-------------------------------------------------------------------
  /** \brief Expose the array of locations. \ingroup API */
  Vector2Djs getX() { return to_js_array(X); }

  /** \brief Expose the array of radii. \ingroup API */
  Vector1Djs getRadii() { return to_js_array(P.r); }

  /** \brief Set the radius of a specific particle. \ingroup API */
  void setRadius(int id, double radius) { P.r[id] = radius; } // NOTE: NOT UPDATING THE MASS!!! THIS IS SUCH A BAD IDEA

  /** \brief Set the mass of a specific particle. \ingroup API */
  void setMass(int id, double mass) { P.m[id] = mass; }

  /** \brief Expose the array of velocities. \ingroup API */
  Vector2Djs getVelocity() { return to_js_array(V); }

  /** \brief Expose the array of orientation rate. \ingroup API */
  Vector1Djs getRotationRate() { Tools<d>::norm(OmegaMag, Omega) ; return to_js_array(OmegaMag); }

  /** \brief DEPRECATED: Use getContactInfo with the appropriate flags instead. Expose the array of particle id and normal forces. \ingroup API */
  Vector2Djs getContactForce() 
  {
    auto [_, res] = MP.contacts2array (ExportData::IDS | ExportData::FN, X, P) ; 
    return to_js_array(res); 
  }
  
  /** \brief Expose the array of contact information. \ingroup API */
  Vector2Djs getContactInfos(int flags) 
  {
    auto [_, res] = MP.contacts2array (static_cast<ExportData>(flags), X, P) ; 
    return to_js_array(res); 
  }
  
  

  // /** \brief DEPRECATED Expose the array of contact forces and velocities. \ingroup API */
  /*std::vector<std::vector<double>> calculateParticleForce()
  {
    std::vector<std::vector<double>> res ;
    std::vector<double> tmpfrc ;
    tmpfrc.resize(2+4*d) ;
    for (size_t i=0 ; i<MP.CLp.size() ; i++)
    {
      for (auto it = MP.CLp[i].v.begin() ; it!=MP.CLp[i].v.end() ; it++)
      {
        if (it->infos == nullptr) continue ;
        tmpfrc[0]=it->i ;
        tmpfrc[1]=it->j ;
        for (int j=0 ; j<d ; j++) tmpfrc[2+j]=it->infos->Fn[j] ;
        for (int j=0 ; j<d ; j++) tmpfrc[2+d+j]=it->infos->Ft[j] ;
        for (int j=0 ; j<d ; j++) tmpfrc[2+2*d+j]=it->infos->vn[j] ;
        for (int j=0 ; j<d ; j++) tmpfrc[2+3*d+j]=it->infos->vt[j] ;
        res.push_back(tmpfrc) ;
      }
    }

    return res;
  }*/

  /** \brief Set the array of locations. \ingroup API */
  //void setX(std::vector < std::vector <double> > X_) { X = X_; }

  /** \brief Set the velocity of a single particle \ingroup API */
  void setVelocity(int id, Vector1Djs vel) {
      auto vel2 = from_js_array(vel) ;
      for (int i=0 ; i<d ; i++) {
          V[id][i] = vel2[i];
      }
  }

  /** \brief Set the angular velocity of a single particle \ingroup API */
  void setAngularVelocity(int id, Vector1Djs omega) {
      auto omega2 = from_js_array(omega) ;
      for (int i=0; i<(d*(d-1)/2); i++) {
          Omega[id][i] = omega2[i];
      }
  }


  /** \brief Set a single particle location, velocity, and angular velocity \ingroup API */
  void fixParticle(int a, Vector1Djs loc) {
      auto loc2 = from_js_array (loc) ;       
      for (int i=0 ; i<d ; i++) {
          X[a][i] = loc2[i];
          V[a][i] = 0;
      }
      for (int i=0; i<(d*(d-1)/2); i++) {
          Omega[a][i] = 0;
      }
  }

  /** \brief Freeze a single particle \ingroup API */
  void setFrozen(int a) {
      P.Frozen[a] = true;
  }

  /** \brief Expose the current time. \ingroup API */
  double getTime() { return t; }

  /** \brief Expose the current gravity angle. \ingroup API */
  double getGravityAngle() { return P.gravityrotateangle; }


  /** \brief Expose the array of orientation. \ingroup API */
  Vector2Djs getOrientation() { return to_js_array(A); }

  /** \brief Expose the array of boundaries. \ingroup API */
  Vector1Djs getBoundary(int a) { return to_js_array(P.Boundaries[a].as_vector()); }
  void setBoundary(int a, Vector1Djs loc) {
      auto loc2 = from_js_array(loc) ;
      P.Boundaries[a].xmin = loc2[0]; // low value
      P.Boundaries[a].xmax = loc2[1]; // high value
      P.Boundaries[a].delta = loc2[1] - loc2[0]; // length
      if ( P.Boundaries[a].Type == WallType::PBC_LE ) {
          P.Boundaries[a].vel = loc2[2];
      }
  }

  /** \brief Expose the array of wall forces. \ingroup API */
  Vector2Djs getWallForce() {
      // std::cout<<P.wallforcerequested<<" "<<P.wallforcecomputed<<std::endl;
      P.wallforcerequested = true;
      if ( P.wallforcecomputed ) {
          P.wallforcerequested = false;
          P.wallforcecomputed = false;
          return to_js_array(WallForce);
      }
      else {
        return to_js_array(empty_array) ;
      }
   }

  /** \brief Set an additional external force on a particle for a certain duration. \param id particle id \param duration number of timesteps to apply the force for \param force force vector to apply \ingroup API */
  void setExternalForce (int id, int duration, Vector1Djs force)
  {
      auto force2 = from_js_array(force) ; 
      
      printf("\nSetting the force: %g %g %g %g\n", force2[0],force2[1],force2[2],force2[3]);
      ExternalAction.resize(ExternalAction.size()+1) ;
      ExternalAction[ExternalAction.size()-1].id = id ;
      ExternalAction[ExternalAction.size()-1].duration = duration ;
      ExternalAction[ExternalAction.size()-1].set(force2, v1d(d,0), v1d(d*(d-1)/2,0), v1d(d*(d-1)/2,0)) ;
  }

  void randomDrop()
  {
    unsigned long int seed = 5489UL ; ///< Seed for the boost RNG. Initialised with the default seed of the Mersenne twister in Boost
    boost::random::mt19937 rng(seed);
    boost::random::uniform_01<boost::mt19937> rand(rng) ;
    for (int i=0 ; i<N ; i++)
        {
         for(int dd=0 ; dd < d ; dd++)
         {
           if (P.Boundaries[dd].Type==WallType::PBC)
             X[i][dd] = rand()*P.Boundaries[dd].delta + P.Boundaries[dd].xmin ;
           else
             X[i][dd] = rand()*(P.Boundaries[dd].delta-2*P.r[i]) + P.Boundaries[dd].xmin + P.r[i] ;
         }
    }
  }

} ;

#ifdef EMSCRIPTEN
#ifdef USEBINDINGS
#include "emscripten_specific.h"
#endif
#endif

/** @} */


/** \mainpage
\section Gen General description
NDDEM is a full suit of tools to perform simulations and visualisions of granular media in any spatial dimension. This documentation gives information on the C++ code base, which has 3 modules. **Dem** is the N-dimensional simulation module, **CoarseGraining** performs coarse-graining of the data from the simulations, **Texturing** handles the rendering as UV maps of the orientation of the hyperspheres, and is designed to be used as an http server in conjunction with the Threejs visualisation module.

\section Prog Program structure

\dot
digraph A {
Input -> Dem ;
Dem -> Results;
Results -> CoarseGraining;
Input -> CoarseGraining;
CoarseGraining -> CGData;
CGData -> Visualisation;
Results -> Texturing;
Texturing -> Textures;
Textures -> Visualisation;
Input -> Visualisation;
Results -> Visualisation;
Input [label="Input File"];
CGData [label="Coarse-grained data"] ;
Dem [shape=box,style=filled,color=".8 1.0 .8"] ;
CoarseGraining [shape=box,style=filled,color=".8 1.0 .8"] ;
Texturing [shape=box,style=filled,color=".8 1.0 .8"] ;
Visualisation [shape=box,style=filled,color="0.9 0.6 0.6"] ;
}
\enddot

\section Prog Examples
\subsection ExDEM DEM Examples
<pre>./DEMND 5 2006 in</pre>: run the simulation with 5 dimensions, 2006 particles, based on the input file named in. Input file example are included in the source, and all commands are documented in the section Input script below.

\subsection ExCG Coarse-Graining Examples
<pre>./IONDDEM in </pre>: run the simulation with 5 dimensions, 2006 particles, based on the input file named in. Input file example are included in the source, and all commands are documented in the section Input script below.

\subsection ExTexturing Texturing Examples
<pre>./TexturingServer </pre>: Run the texturing server for calls from the visualisation side. Typical calling URL are available in the Texturing module.

\section InputScript Input script commands
\subsection Walltypes Wall types
<pre> boundary dim PBC low high </pre> Periodic boundary condition between the boundaries at low and high.<br>
<pre> boundary dim WALL low high </pre> Static walls at boundaries low and high (normal of the wall along the dimension dim). <br>
<pre> boundary dim MOVINGWALL low high vel_low vel_high </pre> Walls moving along their normals<br>
<pre> boundary dim MOVINGSIDEWAYSWALL low high vel_low vel_high </pre> Walls moving along their normals+1 dim<br>
<pre> boundary n SPHERE radius x1 x2 ... xn </pre> Define a sherical wall. n should be higher than the dimension (walls or pbs should be defined in the other dimensions). (xi) is the location of the sphere centre.

*/
