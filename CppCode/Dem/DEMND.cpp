/** \addtogroup DEM Discrete Element Simulations
 * This module handles the Discrete Element Simulations.
 *  @{ */

#include "DEMND.h"
#include <signal.h>
//#include <gperftools/profiler.h>
#include "Benchmark.h"
//#define OMP_NUM_THREADS 2

vector <std::pair<ExportType,ExportData>> * toclean ;
XMLWriter * xmlout ;
void sig_handler (int p)
{
    Benchmark::write_all() ;
    for (auto v : *toclean)
        if ((v.first == ExportType::XML) || (v.first == ExportType::XMLbase64))
            xmlout->emergencyclose() ;
    //printf("\n\n\n\n\n\n\n") ;
    std::exit(p) ;
}

template <int d>
int templatedmain (char * argv[])
{
 int NN=atoi(argv[2]) ;
 Parameters<d> P(NN) ;
 Tools<d>::initialise() ;
 if (!Tools<d>::check_initialised(d)) printf("ERR: Something terribly wrong happened\n") ;
 assert(d<(static_cast<int>(sizeof(int))*8-1)) ; //TODO
 // Array initialisations
 int N=P.N ;
 std::vector < std::vector <double> > X (N, std::vector <double> (d, 0)) ;
 std::vector < std::vector <double> > V (N, std::vector <double> (d, 0)) ;
 std::vector < std::vector <double> > A (N, std::vector <double> (d*d, 0)) ; for (int i=0 ; i<N ; i++) A[i]=Tools<d>::Eye ;
 std::vector < std::vector <double> > Omega (N, std::vector <double> (d*(d-1)/2, 0)) ; //Rotational velocity matrix (antisymetrical)
 std::vector < std::vector <double> > F (N, std::vector <double> (d, 0)) ;
 std::vector < std::vector <double> > FOld (N, std::vector <double> (d, 0)) ;
 std::vector < std::vector <double> > Torque (N, std::vector <double> (d*(d-1)/2, 0)) ;
 std::vector < std::vector <double> > TorqueOld (N, std::vector <double> (d*(d-1)/2, 0)) ;

 std::vector < double > Vmag (N,0) ;
 std::vector < double > OmegaMag (N,0) ;
 std::vector < double > Z (N,0) ;
 std::vector < std::vector <double> > Fcorr (N, std::vector <double> (d, 0)) ;
 std::vector < std::vector <double> > TorqueCorr (N, std::vector <double> (d*(d-1)/2, 0)) ;
 std::vector < double > displacement (N, 0) ; double maxdisp[2] ;

 std::vector <u_int32_t> PBCFlags (N, 0) ;
 std::vector < std::vector <double> > WallForce (2*d, std::vector <double> (d,0)) ;

 vector <u_int32_t> Ghost (N, 0) ;
 vector <u_int32_t> Ghost_dir (N, 0) ;

 v1d Atmp (d*d, 0) ;

 // Initial state setup
 P.set_boundaries() ;
 //P.init_particles(X, A) ;
 if (strcmp(argv[3], "default"))
     P.load_datafile (argv[3], X, V, Omega) ;
 toclean = &(P.dumps) ;
 xmlout = P.xmlout ;

 displacement[0]=P.skinsqr*2 ;
 //Contacts C(P) ; //Initialize the Contact class object
 //ContactList CLp, CLw ;
 const char* env_p = std::getenv("OMP_NUM_THREADS") ;
 int numthread = 2 ;
 if (env_p!=nullptr) numthread = atoi (env_p) ;
 omp_set_num_threads(numthread) ;
 Multiproc<d> MP(N, numthread, P) ;

 clock_t tnow, tprevious ; tprevious=clock() ;
 double t ; int ti ;
 double dt=P.dt ;
 FILE *logfile = fopen("Logfile", "w") ;

//ProfilerStart("Profiling") ;
printf("[INFO] Orientation tracking is %s\n", P.orientationtracking?"True":"False") ;
 for (t=0, ti=0 ; t<P.T ; t+=dt, ti++)
 {
   //bool isdumptime = (ti % P.tdump==0) ;
   //P.display_info(ti, V, Omega, F, Torque, 0, 0) ;
   if (ti%P.tinfo==0)
   {
     tnow = clock();
     printf("\r%10g | %5.2g%% | %d iterations in %10gs | %5d | finish in %10gs",t, t/P.T*100, P.tinfo,
            double(tnow - tprevious) / CLOCKS_PER_SEC, ti, ((P.T-t)/(P.tinfo*dt))*(double(tnow - tprevious) / CLOCKS_PER_SEC)) ;
     fprintf(logfile, "%d %10g %lu %lu\n", ti, double(tnow - tprevious) / CLOCKS_PER_SEC, MP.CLp[0].v.size(), MP.CLw[0].v.size()) ;
     fflush(stdout) ;
     tprevious=tnow ;
   }

   //----- Velocity Verlet step 1 : compute the new positions
   Benchmark::start_clock("Verlet 1st");
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

    /*Tools<d>::skewexpand(tmpO, Omega[i]) ;
    Tools<d>::matmult (tmpterm1, tmpO, A[i]);
    Tools<d>::skewexpand(tmpT, TorqueOld[i]) ;
    Tools<d>::skewmatsquare(tmpOO,Omega[i]) ;
    Tools<d>::vScaledSum (tmpSUM , 1/P.I[i], tmpT, tmpOO) ;
    Tools<d>::matmult(tmpterm2, tmpSUM, A[i]) ;
    for (int dd=0 ; dd<d *d ; dd++)
        A[i][dd] += tmpterm1[dd]*dt + tmpterm2[dd] *dt*dt/2. ;*/

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
    u_int32_t mask=1 ;

    for (int j=0 ; j<d ; j++, mask<<=1)
    {
     if (P.Boundaries[j][3] != static_cast<int>(WallType::PBC)) continue ;
     if      (X[i][j] <= P.Boundaries[j][0] + P.skin) {Ghost[i] |= mask ; }
     else if (X[i][j] >= P.Boundaries[j][1] - P.skin) {Ghost[i] |= mask ; Ghost_dir[i] |= mask ;}
    }
    //Nghosts=Ghosts.size() ;
  } // END PARALLEL SECTION
   P.perform_MOVINGWALL() ;
   Benchmark::stop_clock("Verlet 1st");

   //---------- Velocity Verlet step 2 : compute the forces and torques
   // Contact detection (sequential, can be paralelised easily with openmp)

   Benchmark::start_clock("Contacts");

   bool recompute = true ;
   // Should we recompute the neighbor list?
   //auto res=Tools<d>::two_max_element(displacement) ;
   //if (maxdisp[0]+maxdisp[1] > 0.7*(P.skin-P.r[0]*2)) {recompute=true ; std::fill(displacement.begin(), displacement.end(), 0);}
   //else recompute=false ;
   if (recompute)
   {
     //printf("RECOMPUTE\n");
     // fflush(stdout) ;
     #pragma omp parallel default(none) shared(MP) shared(P) shared(N) shared(X) shared(Ghost) shared(Ghost_dir) //shared (stdout)
     {
       int ID = omp_get_thread_num();
       ContactList<d> & CLp = MP.CLp[ID] ; ContactList<d> & CLw = MP.CLw[ID] ;
       cp tmpcp(0,0,d,0,nullptr) ; double sum=0 ;
       CLp.reset() ; CLw.reset();

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
       }
       CLp.finalise() ;
       CLw.finalise() ;
     } //END PARALLEL SECTION
   }
   else // Do not recompute the full contact list, but still compute the contact length and all.
   {
     /*#pragma omp parallel default(none) shared(MP) shared(P)  shared(d) shared(X) shared(Ghost) shared(Ghost_dir) //shared(stdout)
     {
       int ID = omp_get_thread_num();
       double sum=0 ;
       ContactList & CLp = MP.CLp[ID] ; ContactList & CLw = MP.CLw[ID] ;

       for (auto it = CLp.v.begin() ; it != CLp.v.end() ; it++)
       {
         sum=0 ;
         for (int k=0 ; k<d ; k++) sum+= (X[it->i][k]-X[it->j][k])*(X[it->i][k]-X[it->j][k]) ;
         it->contactlength=sum ; it->ghost=0 ; it->ghostdir=0 ; //WARNING: the contactlength is squared temporarily, to avoid computing unnecessary sqrt.
         if (Ghost[it->j])
         {
           it->ghostdir=Ghost_dir[it->j] ;
           CLp.check_ghost_dst(Ghost[it->j], 0, sum, 0, P, X[it->i], X[it->j], *it) ;
         }
         it->contactlength=sqrt(it->contactlength) ; // Final squarerooting
       }

       for (auto it = CLw.v.begin() ; it != CLw.v.end() ; it++)
       {
         int w = it->j / 2, wdir=it->j % 2 ;
         it->contactlength=fabs(X[it->i][w]-P.Boundaries[w][wdir]) ;
       }
     }*/
     printf("NOT IMPLEMENTED\n") ;
   }
   Benchmark::stop_clock("Contacts");

   //-------------------------------------------------------------------------------
   // Force and torque computation
   Benchmark::start_clock("Forces");
   Tools<d>::setzero(F) ; Tools<d>::setzero(Fcorr) ; Tools<d>::setzero(TorqueCorr) ;
   Tools<d>::setgravity(F, P.g, P.m); // Actually set gravity is effectively also doing the setzero
   Tools<d>::setzero(Torque);

   //Particle - particle contacts
   #pragma omp parallel default(none) shared(MP) shared(P) shared(X) shared(V) shared(Omega) shared(F) shared(Fcorr) shared(TorqueCorr) shared(Torque) //shared(stdout)
   {
     int ID = omp_get_thread_num();
     ContactList<d> & CLp = MP.CLp[ID] ; ContactList<d> & CLw = MP.CLw[ID] ; Contacts<d> & C =MP.C[ID] ;

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

     for (auto it = CLw.v.begin() ; it!=CLw.v.end() ; it++)
     {
      C.particle_wall( V[it->i],Omega[it->i],P.r[it->i], it->j/2, (it->j%2==0)?-1:1, *it) ;
      //Tools<d>::vAdd(F[it->i], Act.Fn, Act.Ft) ; // F[it->i] += (Act.Fn+Act.Ft) ;
      //Torque[it->i] += Act.Torquei ;

      Tools<d>::vAddFew(F[it->i], C.Act.Fn, C.Act.Ft, Fcorr[it->i]) ;
      Tools<d>::vAddOne(Torque[it->i], C.Act.Torquei, TorqueCorr[it->i]) ;

      if (P.wallforcecompute) MP.delayingwall(ID, it->j, C.Act) ;
     }
   }

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

   Benchmark::stop_clock("Forces");

   //---------- Velocity Verlet step 3 : compute the new velocities
   Benchmark::start_clock("Verlet last");
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

   Benchmark::stop_clock("Verlet last");

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
     if (P.wallforcecompute)
     {
       for (int i=0 ; i<MP.P ; i++)
           for (uint j=0 ; j<MP.delayedwall_size[i] ; j++)
               Tools<d>::vSubFew(WallForce[MP.delayedwallj[i][j]], MP.delayedwall[i][j].Fn, MP.delayedwall[i][j].Ft) ;
     }
     Tools<d>::savetxt(path, WallForce, ( char const *)("Force on the various walls")) ;
    }
   }

   if (P.wallforcecompute) MP.delayedwall_clean() ;
 }

//ProfilerStop() ;
//Tools<d>::write1D ("Res.txt", TmpRes) ;
//Tools<d>::writeinline_close() ;
Benchmark::write_all();
P.finalise() ;
printf("This is the end ...\n") ;
fclose(logfile) ;
return 0 ;
}


//===================================================
int main (int argc, char *argv[])
{
 signal (SIGINT, sig_handler);   // Catch all signals ...

 if (argc<4) {printf("Usage: DEMND #dimensions #grains inputfile\n") ; std::exit(1) ; }
 int dd=atoi(argv[1]) ;

 switch (dd)
 {
     case  1: templatedmain<1> (argv) ; break ;
     case  2: templatedmain<2> (argv) ; break ;
     case  3: templatedmain<3> (argv) ; break ;
     case  4: templatedmain<4> (argv) ; break ;
     case  5: templatedmain<5> (argv) ; break ;
     case  6: templatedmain<6> (argv) ; break ;
     case  7: templatedmain<7> (argv) ; break ;
     case  8: templatedmain<8> (argv) ; break ;
//      case  9: templatedmain<9> (argv) ; break ;
//      case 10: templatedmain<10> (argv) ; break ;
//      case 11: templatedmain<11> (argv) ; break ;
//      case 12: templatedmain<12> (argv) ; break ;
//      case 13: templatedmain<13> (argv) ; break ;
//      case 14: templatedmain<14> (argv) ; break ;
//      case 15: templatedmain<15> (argv) ; break ;
//      case 16: templatedmain<16> (argv) ; break ;
//      case 17: templatedmain<17> (argv) ; break ;
//      case 18: templatedmain<18> (argv) ; break ;
//      case 19: templatedmain<19> (argv) ; break ;
//      case 20: templatedmain<20> (argv) ; break ;
//      case 21: templatedmain<21> (argv) ; break ;
//      case 22: templatedmain<22> (argv) ; break ;
//      case 23: templatedmain<23> (argv) ; break ;
//      case 24: templatedmain<24> (argv) ; break ;
//      case 25: templatedmain<25> (argv) ; break ;
     default : printf("DEMND was not compiled with support for dimension %d. Please recompile modifying the main function to support that dimension.\n", dd); std::exit(1) ;
 }

return 0 ;
}

/** @} */
