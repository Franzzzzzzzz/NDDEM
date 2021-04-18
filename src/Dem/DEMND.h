#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <vector>
#include <ctime>
#include <cstring>
#ifndef NO_OPENMP
    #include <omp.h>
#endif

#include "Typedefs.h"
#include "Parameters.h"
#include "Contacts.h"
#include "ContactList.h"
#include "Multiproc.h"
#ifdef EMSCRIPTEN
    #include <emscripten.h>
    #include <emscripten/bind.h>
    using namespace emscripten;
#endif


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
    std::vector < double > displacement ; double maxdisp[2] ;

    std::vector <uint32_t> PBCFlags ;
    std::vector < std::vector <double> > WallForce ;

    vector <uint32_t> Ghost ;
    vector <uint32_t> Ghost_dir ;
    v1d Atmp ;

    int numthread=1 ;
    Multiproc<d> MP ;

    double t ; int ti ;
    double dt ;
    clock_t tnow, tprevious ;

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
        displacement.resize (N, 0) ;

        PBCFlags.resize (N, 0) ;
        WallForce.resize (2*d, std::vector <double> (d,0)) ;

        Ghost.resize (N, 0) ;
        Ghost_dir.resize (N, 0) ;

        Atmp.resize (d*d, 0) ;

        // Initial state setup
        P.set_boundaries() ;
    }
    //-------------------------------------------------------------------------
    void init_from_file (char filename[])
    {
      P.load_datafile (filename, X, V, Omega) ;
    }
    //-------------------------------------------------------------------------
    void finalise_init () {
        //if (strcmp(argv[3], "default"))
        //    P.load_datafile (argv[3], X, V, Omega) ;
        toclean = &(P.dumps) ;
        xmlout = P.xmlout ;

        displacement[0]=P.skinsqr*2 ;

        #ifndef NO_OPENMP
          const char* env_p = std::getenv("OMP_NUM_THREADS") ;
          if (env_p!=nullptr) numthread = atoi (env_p) ;
          omp_set_num_threads(numthread) ;
        #endif

        MP.initialise(N, numthread, P) ;

        dt=P.dt ;
        t=0 ; ti=0 ;
        tprevious=clock() ;
        printf("[INFO] Orientation tracking is %s\n", P.orientationtracking?"True":"False") ;
    }
    //-------------------------------------------------------------------
    void interpret_command (string in)
    {
        P.interpret_command(in, X,V,Omega) ;
    }
    //--------------------------------------------------------------------
    void step_forward ()
    {
      int nsteps = P.T/dt ;
      step_forward(nsteps) ;
    }
    //----
    void step_forward (int nt)
    {
      for (int ntt=0 ; ntt<nt ; ntt++, t+=dt, ti++)
      {
        // printf("UP TO TIME: %g with timestep: %g\n", t, dt);
        // printf("%g %g %g\n", X[0][0],X[0][1],X[0][2]);
        //bool isdumptime = (ti % P.tdump==0) ;
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
        maxdisp[0] = 0 ; maxdisp[1] = 0 ;
        #pragma omp parallel for default(none) shared (N) shared(X) shared(P) shared(V) shared(FOld) shared(Omega) shared(PBCFlags) shared(dt) shared(Ghost) shared(Ghost_dir) shared(A) shared(maxdisp) shared(displacement) //ERROR RACE CONDITION ON MAXDISP
        for (int i=0 ; i<N ; i++)
        {
            double disp, totdisp=0 ;
            for (int dd=0 ; dd<d ; dd++)
            {
                disp = V[i][dd]*dt + FOld[i][dd] * (dt * dt / P.m[i] /2.) ;
                X[i][dd] += disp  ;
                totdisp += disp*disp ;
            }
            displacement[i] += sqrt(totdisp) ;
            if (displacement[i] > maxdisp[0]) {maxdisp[1]=maxdisp[0] ; maxdisp[0]=displacement[i] ; } // ERROR RACE CONDITION ON MAXDISP

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

            // Find ghosts
            Ghost[i]=0 ; Ghost_dir[i]=0 ;
            uint32_t mask=1 ;

            for (int j=0 ; j<d ; j++, mask<<=1)
            {
            if (P.Boundaries[j][3] != static_cast<int>(WallType::PBC)) continue ;
            if      (X[i][j] <= P.Boundaries[j][0] + P.skin) {Ghost[i] |= mask ; }
            else if (X[i][j] >= P.Boundaries[j][1] - P.skin) {Ghost[i] |= mask ; Ghost_dir[i] |= mask ;}
            }
            //Nghosts=Ghosts.size() ;
        } // END PARALLEL SECTION
        P.perform_MOVINGWALL() ;

        #pragma omp parallel default(none) shared(MP) shared(P) shared(N) shared(X) shared(Ghost) shared(Ghost_dir) //shared (stdout)
        {
         //int ID = 0 ; //OMP TODO
         #ifdef NO_OPENMP
         int ID = 0 ;
         #else
         int ID = omp_get_thread_num();
         double timebeg = omp_get_wtime();
         #endif
         ContactList<d> & CLp = MP.CLp[ID] ; 
         ContactList<d> & CLw = MP.CLw[ID] ;
         ContactList<d> & CLb = MP.CLb[ID] ; 
         CLp.reset() ; CLw.reset(); CLb.reset() ;
         cp tmpcp(0,0,d,0,nullptr) ; double sum=0 ; 

         for (int i=MP.share[ID] ; i<MP.share[ID+1] ; i++)
         {
            tmpcp.setinfo(CLp.default_action());
            tmpcp.i=i ;
            for (int j=i+1 ; j<N ; j++) // Regular particles
            {
                if (Ghost[j])
                {
                    tmpcp.j=j ; tmpcp.ghostdir=Ghost_dir[j] ;
                    CLp.check_ghost (Ghost[j], P, X[i], X[j], tmpcp) ;
                }
                else
                {
                    sum=0 ;
                    for (int k=0 ; sum<P.skinsqr && k<d ; k++) sum+= (X[i][k]-X[j][k])*(X[i][k]-X[j][k]) ;
                    if (sum<P.skinsqr)
                    {
                        tmpcp.j=j ; tmpcp.contactlength=sqrt(sum) ; tmpcp.ghost=0 ; tmpcp.ghostdir=0 ;
                        CLp.insert(tmpcp) ;
                    }
                }
            }

            tmpcp.setinfo(CLw.default_action());
            tmpcp.i=i ;
            for (int j=0 ; j<d ; j++) // Wall contacts
            {
                    if (P.Boundaries[j][3]==static_cast<int>(WallType::PBC)) continue ;

                    tmpcp.contactlength=fabs(X[i][j]-P.Boundaries[j][0]) ;
                    if (tmpcp.contactlength<P.skin)
                    {
                        tmpcp.j=(2*j+0);
                        CLw.insert(tmpcp) ;
                    }

                    tmpcp.contactlength=fabs(X[i][j]-P.Boundaries[j][1]) ;
                    if (tmpcp.contactlength<P.skin)
                    {
                        tmpcp.j=(2*j+1);
                        CLw.insert(tmpcp) ;
                    }
            }
            
            tmpcp.setinfo(CLb.default_action());
            tmpcp.i=i ;
            for (size_t j=0 ; j<P.body.size() ; j++) // Facet contacts
            {
                CLw.check_facet_dst (P, X[i], j, tmpcp) ; // Contact insertion handled by the subroutine
            }
            
        }
        CLp.finalise() ;
        CLw.finalise() ;
        CLb.finalise() ;
        #ifndef NO_OPENMP
        MP.timing[ID] += omp_get_wtime()-timebeg;
        #endif
      } //END PARALLEL SECTION
        //-------------------------------------------------------------------------------
        // Force and torque computation
        Tools<d>::setzero(F) ; Tools<d>::setzero(Fcorr) ; Tools<d>::setzero(TorqueCorr) ;
        Tools<d>::setgravity(F, P.g, P.m); // Actually set gravity is effectively also doing the setzero
        Tools<d>::setzero(Torque);

        //Particle - particle contacts
        #pragma omp parallel default(none) shared(MP) shared(P) shared(X) shared(V) shared(Omega) shared(F) shared(Fcorr) shared(TorqueCorr) shared(Torque) //shared(stdout)
        {
          #ifdef NO_OPENMP
            int ID = 0 ;
          #else
            int ID = omp_get_thread_num();
            double timebeg = omp_get_wtime();
          #endif
            ContactList<d> & CLp = MP.CLp[ID] ; ContactList<d> & CLw = MP.CLw[ID] ; ContactList<d> & CLb = MP.CLb[ID] ;
            Contacts<d> & C =MP.C[ID] ;

            for (auto it = CLp.v.begin() ; it!=CLp.v.end() ; it++)
            {
            if (it->ghost==0)
            {
                C.particle_particle(X[it->i], V[it->i], Omega[it->i], P.r[it->i],
                                        X[it->j], V[it->j], Omega[it->j], P.r[it->j], *it) ;
            }
            else
            {
                C.particle_ghost(X[it->i], V[it->i], Omega[it->i], P.r[it->i],
                                    X[it->j], V[it->j], Omega[it->j], P.r[it->j], *it) ;
            }

            Tools<d>::vAddFew(F[it->i], C.Act.Fn, C.Act.Ft, Fcorr[it->i]) ;
            Tools<d>::vAddOne(Torque[it->i], C.Act.Torquei, TorqueCorr[it->i]) ;

            if (MP.ismine(ID,it->j))
            {
                Tools<d>::vSubFew(F[it->j], C.Act.Fn, C.Act.Ft, Fcorr[it->j]) ;
                Tools<d>::vAddOne(Torque[it->j], C.Act.Torquej, TorqueCorr[it->j]) ;
            }
            else
                MP.delaying(ID, it->j, C.Act) ;

            //Tools<d>::vAdd(F[it->i], Act.Fn, Act.Ft) ; Tools<d>::vSub(F[it->j], Act.Fn, Act.Ft) ; //F[it->i] += (Act.Fn + Act.Ft) ; F[it->j] -= (Act.Fn + Act.Ft) ;
            //Torque[it->i] += Act.Torquei ; Torque[it->j] += Act.Torquej ;
            }

            //------
            for (auto it = CLw.v.begin() ; it!=CLw.v.end() ; it++)
            {
            C.particle_wall( V[it->i],Omega[it->i],P.r[it->i], it->j/2, (it->j%2==0)?-1:1, *it) ;
            //Tools<d>::vAdd(F[it->i], Act.Fn, Act.Ft) ; // F[it->i] += (Act.Fn+Act.Ft) ;
            //Torque[it->i] += Act.Torquei ;

            Tools<d>::vAddFew(F[it->i], C.Act.Fn, C.Act.Ft, Fcorr[it->i]) ;
            Tools<d>::vAddOne(Torque[it->i], C.Act.Torquei, TorqueCorr[it->i]) ;

            if (P.wallforcecompute) MP.delayingwall(ID, it->j, C.Act) ;
            }
            
            //------
            for (auto it = CLb.v.begin() ; it!=CLb.v.end(); it++)
            {
                C.particle_facet(V[it->i],Omega[it->i],P.r[it->i], P.body[it->j].edges[0], *it) ; 
                Tools<d>::vAddFew(F[it->i], C.Act.Fn, C.Act.Ft, Fcorr[it->i]) ;
                Tools<d>::vAddOne(Torque[it->i], C.Act.Torquei, TorqueCorr[it->i]) ;
            }
            
            #ifndef NO_OPENMP
            MP.timing[ID] += omp_get_wtime()-timebeg;
            #endif
        } //END PARALLEL PART

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
        #pragma omp parallel for default(none) shared(N) shared(P) shared(V) shared(Omega) shared(F) shared(FOld) shared(Torque) shared(TorqueOld) shared(dt)
        for (int i=0 ; i<N ; i++)
        {
            //printf("%10g %10g %10g\n%10g %10g %10g\n%10g %10g %10g\n\n", A[0][0], A[0][1], A[0][2], A[0][3], A[0][4], A[0][5], A[0][6], A[0][7], A[0][8]) ;
            if (P.Frozen[i]) {Tools<d>::setzero(TorqueOld[i]) ; Tools<d>::setzero(F[i]) ; Tools<d>::setzero(FOld[i]) ; Tools<d>::setzero(V[i]) ; Tools<d>::setzero(Omega[i]) ; }

            Tools<d>::vAddScaled(V[i], dt/2./P.m[i], F[i], FOld[i]) ; //V[i] += (F[i] + FOld[i])*(dt/2./P.m[i]) ;
            Tools<d>::vAddScaled(Omega[i], dt/2./P.I[i], Torque[i], TorqueOld[i]) ; // Omega[i] += (Torque[i]+TorqueOld[i])*(dt/2./P.I[i]) ;
            FOld[i]=F[i] ;
            TorqueOld[i]=Torque[i] ;
        } // END OF PARALLEL SECTION

        // Benchmark::stop_clock("Verlet last");

        // Check events
        P.check_events(t, X,V,Omega) ;

        // Output something at some point I guess
        if (ti % P.tdump==0)
        {
            Tools<d>::setzero(Z) ; for (auto &v: MP.CLp) v.coordinance(Z) ;
            P.dumphandling (ti, t, X, V, Vmag, A, Omega, OmegaMag, PBCFlags, Z) ;
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
        }

        if (P.wallforcecompute) MP.delayedwall_clean() ;

        // Load balancing on the procs as needed
        #ifndef NO_OPENMP
        MP.num_time++ ;
        if (MP.num_time>100)
        {
          MP.load_balance() ;
          // Cleaning the load balancing
          MP.num_time = 0 ;
          MP.timing = vector<double>(MP.P,0) ;
        }
        #endif
    }
  }

  //-------------------------
  void finalise ()
  {
    P.finalise() ;
    printf("This is the end ...\n") ;
    //fclose(logfile) ;
  }

  //-------------------------------------------------------------------
  std::vector<std::vector<double>> getX() { return X; }
  void setX(std::vector < std::vector <double> > X_) { X = X_; }
  void fixParticle(int a, std::vector<double> loc) {
      X[a][0] = loc[0];
      X[a][1] = loc[1];
      X[a][2] = loc[2];
      V[a][0] = 0;
      V[a][1] = 0;
      V[a][2] = 0;
      for (int i=0; i<(d*(d-1)/2); i++) {
          Omega[a][i] = 0;
      }
  }

  std::vector<std::vector<double>> getOrientation() { return A; }

  std::vector<double> getBoundary(int a) { return P.Boundaries[a]; }
  void setBoundary(int a, std::vector<double> loc) {
      P.Boundaries[a][0] = loc[0]; // low value
      P.Boundaries[a][1] = loc[1]; // high value
      P.Boundaries[a][2] = loc[1] - loc[0]; // length
  }

  std::vector<std::vector<double>> getWallForce() { return WallForce; }
  std::vector<std::vector<double>> getVelocity() { return V; }

} ;

#ifdef EMSCRIPTEN
EMSCRIPTEN_BINDINGS(my_class_example) {
    class_<Simulation<3>>("Simulation3")
        .constructor<int>()
        .function("finalise_init", &Simulation<3>::finalise_init)
        .function("interpret_command", &Simulation<3>::interpret_command)
        .function("step_forward", &Simulation<3>::step_forward)
        .function("finalise", &Simulation<3>::finalise)
        // .smart_ptr<std::shared_ptr<Simulation<3>>>("Simulation")
        // .property("X", &Simulation<3>::getX, &Simulation<3>::setX)
        .function("getX", &Simulation<3>::getX)
        .function("fixParticle", &Simulation<3>::fixParticle)
        .function("getOrientation", &Simulation<3>::getOrientation)
        .function("getVelocity", &Simulation<3>::getVelocity)
        // .function("getX2", &Simulation<3>::getX2)
        .function("getBoundary", &Simulation<3>::getBoundary)
        .function("setBoundary", &Simulation<3>::setBoundary)
        .function("getWallForce", &Simulation<3>::getWallForce)
        ;
    class_<Simulation<4>>("Simulation4")
        .constructor<int>()
        .function("finalise_init", &Simulation<4>::finalise_init)
        .function("interpret_command", &Simulation<4>::interpret_command)
        .function("step_forward", &Simulation<4>::step_forward)
        .function("finalise", &Simulation<4>::finalise)
        // .smart_ptr<std::shared_ptr<Simulation<3>>>("Simulation")
        // .property("X", &Simulation<3>::getX, &Simulation<3>::setX)
        .function("getX", &Simulation<4>::getX)
        .function("fixParticle", &Simulation<4>::fixParticle)
        .function("getOrientation", &Simulation<4>::getOrientation)
        .function("getVelocity", &Simulation<4>::getVelocity)
        // .function("getX2", &Simulation<3>::getX2)
        .function("getBoundary", &Simulation<4>::getBoundary)
        .function("setBoundary", &Simulation<4>::setBoundary)
        .function("getWallForce", &Simulation<4>::getWallForce)
        ;

}

// EMSCRIPTEN_BINDINGS(stl_wrappers) {
//     emscripten::register_vector<double>("Vec1DDouble");
//     emscripten::register_vector<std::vector<double>>("Vec2DDouble");
// }

namespace emscripten {
namespace internal {

template <typename T, typename Allocator>
struct BindingType<std::vector<T, Allocator>> {
    using ValBinding = BindingType<val>;
    using WireType = ValBinding::WireType;

    static WireType toWireType(const std::vector<T, Allocator> &vec) {
        return ValBinding::toWireType(val::array(vec));
    }

    static std::vector<T, Allocator> fromWireType(WireType value) {
        return vecFromJSArray<T>(ValBinding::fromWireType(value));
    }
};

template <typename T>
struct TypeID<T,
              typename std::enable_if_t<std::is_same<
                  typename Canonicalized<T>::type,
                  std::vector<typename Canonicalized<T>::type::value_type,
                              typename Canonicalized<T>::type::allocator_type>>::value>> {
    static constexpr TYPEID get() { return TypeID<val>::get(); }
};

}  // namespace internal
}  // namespace emscripten
#endif




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


*/
