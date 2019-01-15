#include "DEMND.h"
#include <signal.h>
#include "Benchmark.h"
#define OMP_NUM_THREADS 2

uint Tools::d=0 ;
vector < vector <int> > Tools::MSigns ;
vector < vector <int> > Tools::MIndexAS ;
vector < double > Tools::Eye ;
vector < pair <int,int> > Tools::MASIndex ;
vector <FILE *> Tools::outs ;
boost::random::mt19937 Tools::rng ;
boost::random::uniform_01<boost::mt19937> Tools::rand(rng) ;

void sig_handler (int p)
{
    Benchmark::write_all() ;
    //printf("\n\n\n\n\n\n\n") ;
    std::exit(p) ;
}


int main (int argc, char *argv[])
{
 signal (SIGINT, sig_handler);   // Catch all signals ...

 if (argc<4) {printf("Usage: DEMND #dimensions #grains inputfile\n") ; std::exit(1) ; }
 int dd=atoi(argv[1]) ; int NN=atoi(argv[2]) ;
 Parameters P(dd,NN) ;
 Tools::initialise(P.d) ;
 if (!Tools::check_initialised(P.d)) printf("ERR: Something terribly wrong happened\n") ;
 assert(P.d<(sizeof(int)*8-1)) ;
 // Array initialisations
 int N=P.N ; int d=P.d ;
 std::vector < std::vector <double> > X (N, std::vector <double> (d, 0)) ;
 std::vector < std::vector <double> > V (N, std::vector <double> (d, 0)) ;
 std::vector < std::vector <double> > A (N, std::vector <double> (d*d, 0)) ; for (int i=0 ; i<N ; i++) A[i]=Tools::Eye ;
 std::vector < std::vector <double> > Omega (N, std::vector <double> (d*(d-1)/2, 0)) ; //Rotational velocity matrix (antisymetrical)
 std::vector < std::vector <double> > F (N, std::vector <double> (d, 0)) ;
 std::vector < std::vector <double> > FOld (N, std::vector <double> (d, 0)) ;
 std::vector < std::vector <double> > Torque (N, std::vector <double> (d*(d-1)/2, 0)) ;
 std::vector < std::vector <double> > TorqueOld (N, std::vector <double> (d*(d-1)/2, 0)) ;

 std::vector < std::vector <double> > Fcorr (N, std::vector <double> (d, 0)) ;
 std::vector < std::vector <double> > TorqueCorr (N, std::vector <double> (d*(d-1)/2, 0)) ;

 vector <u_int32_t> Ghost (N, 0) ;
 vector <u_int32_t> Ghost_dir (N, 0) ;

 v1d Atmp (d*d, 0) ;
 v1d tmpO (d*d,0), tmpT (d*d,0), tmpOO (d*d,0), tmpSUM (d*d,0),  tmpterm1 (d*d,0), tmpterm2 (d*d,0)  ;

 // Initial state setup
 P.set_boundaries() ;
 //P.init_particles(X, A) ;
 if (strcmp(argv[3], "default"))
     P.load_datafile (argv[3], X, V, Omega) ;
 if (P.dumpkind==ExportType::XML || P.dumpkind==ExportType::XMLbase64) P.xmlout->header(d, argv[3]) ;

 //Contacts C(P) ; //Initialize the Contact class object
 //ContactList CLp, CLw ;
 omp_set_num_threads(OMP_NUM_THREADS) ;
 Multiproc MP(N, OMP_NUM_THREADS, P) ;

 clock_t tnow, tprevious ; tprevious=clock() ;
 double t ; int ti ;
 double dt=P.dt ;

 for (t=0, ti=0 ; t<P.T ; t+=dt, ti++)
 {
   //bool isdumptime = (ti % P.tdump==0) ;
   if (ti%P.tinfo==0)
   {
     tnow = clock();
     printf("\r%10g | %5.2g%% | %d iterations in %10gs | %5d | finish in %10gs",t, t/P.T*100, P.tinfo,
            double(tnow - tprevious) / CLOCKS_PER_SEC, ti, ((P.T-t)/(P.tinfo*dt))*(double(tnow - tprevious) / CLOCKS_PER_SEC)) ;
     fflush(stdout) ;
     tprevious=tnow ;
   }

   //----- Velocity Verlet step 1 : compute the new positions
   Benchmark::start_clock("Verlet 1st");
   for (int i=0 ; i<N ; i++)
   {
    for (int dd=0 ; dd<d ; dd++)
        X[i][dd] += V[i][dd]*dt + FOld[i][dd] * (dt * dt / P.m[i] /2.) ;


    Tools::skewexpand(tmpO, Omega[i]) ;
    Tools::matmult (tmpterm1, tmpO, A[i]);
    Tools::skewexpand(tmpT, TorqueOld[i]) ;
    Tools::skewmatsquare(tmpOO,Omega[i]) ;
    Tools::vScaledSum (tmpSUM , 1/P.I[i], tmpT, tmpOO) ;
    Tools::matmult(tmpterm2, tmpSUM, A[i]) ;

    for (int dd=0 ; dd<d *d ; dd++)
        A[i][dd] += tmpterm1[dd]*dt + tmpterm2[dd] *dt*dt/2. ;

    P.perform_PBC(X[i]) ;

    // Find ghosts
    Ghost[i]=0 ; Ghost_dir[i]=0 ;
    u_int32_t mask=1 ;
    for (int j=0 ; j<d ; j++, mask<<=1)
    {
     if (P.Boundaries[j][3] != 0) continue ;
     //if      (X[i][j] < P.Boundaries[j][0] + 2*P.r[i]) { tmpghost[0]=i ; tmpghost[1]=j ; Ghosts.push_back(tmpghost) ; Ghosts_deltas.push_back( P.Boundaries[j][2]) ; }
     //else if (X[i][j] > P.Boundaries[j][1] - 2*P.r[i]) { tmpghost[0]=i ; tmpghost[1]=j ; Ghosts.push_back(tmpghost) ; Ghosts_deltas.push_back(-P.Boundaries[j][2]) ; }
     if      (X[i][j] <= P.Boundaries[j][0] + 2*P.r[i]) {Ghost[i] |= mask ; } //Ghost_dir [i] |= mask*0 ;
     else if (X[i][j] >= P.Boundaries[j][1] - 2*P.r[i]) {Ghost[i] |= mask ; Ghost_dir[i] |= mask ;}
    }
    //Nghosts=Ghosts.size() ;
   }
   Benchmark::stop_clock("Verlet 1st");

   //---------- Velocity Verlet step 2 : compute the forces and torques
   // Contact detection (sequential, can be paralelised easily with openmp)

   Benchmark::start_clock("Contacts");

   #pragma omp parallel default(none) shared(MP) shared(P) shared(d) shared(N) shared(X) shared(Ghost) shared(Ghost_dir) shared (stdout)
   {
     int ID = omp_get_thread_num();
     ContactList & CLp = MP.CLp[ID] ; ContactList & CLw = MP.CLw[ID] ;
     cp tmpcp(0,0,d,0,nullptr) ; double sum=0 ;
     CLp.reset() ; CLw.reset();

     for (int i=MP.share[ID] ; i<MP.share[ID+1] ; i++)
     {
         //Benchmark::start_clock("Contacts_part");
         tmpcp.setinfo(CLp.default_action());


         for (int j=i+1 ; j<N ; j++) // Regular particles
         {
             sum=0 ;
             for (int k=0 ; k<d ; k++) sum+= (X[i][k]-X[j][k])*(X[i][k]-X[j][k]) ;
             if (sum<(P.r[i]+P.r[j])*(P.r[i]+P.r[j]))
             {
                 tmpcp.i=i ; tmpcp.j=j ; tmpcp.contactlength=sqrt(sum) ; tmpcp.ghost=0 ; tmpcp.ghostdir=0 ;
                 CLp.insert(tmpcp) ;

                 //if (CLp.cid>345090 && CLp.cid<345110 && i==0 && j==1) printf("#n %d %d %g %g\n", i, j, X[i][0], X[j][0]) ;
             }

             if (Ghost[j])
             {
               tmpcp.i=i ; tmpcp.j=j ; tmpcp.ghostdir=Ghost_dir[j] ;
               CLp.check_ghost(Ghost[j], 0, sum, 0, P, X[i], X[j], (P.r[i]+P.r[j])*(P.r[i]+P.r[j]), tmpcp) ;
             }
           }
         //Benchmark::stop_clock("Contacts_part");

         //Benchmark::start_clock("Contacts_bound");
         tmpcp.setinfo(CLw.default_action());
         for (int j=0 ; j<d ; j++) // Wall contacts
         {
              if (P.Boundaries[j][3]!=1) continue ;

              tmpcp.contactlength=fabs(X[i][j]-P.Boundaries[j][0]) ;
              if (tmpcp.contactlength<P.r[i])
              {
                  tmpcp.i=i ; tmpcp.j=(2*j+0);
                  CLw.insert(tmpcp) ;
              }

              tmpcp.contactlength=fabs(X[i][j]-P.Boundaries[j][1]) ;
              if (tmpcp.contactlength<P.r[i])
              {
                  tmpcp.i=i ; tmpcp.j=(2*j+1);
                  CLw.insert(tmpcp) ;
              }
         }
         //Benchmark::stop_clock("Contacts_bound");
     }
     CLp.finalise() ;
     CLw.finalise() ;
   } //END PARALLEL SECTION
   Benchmark::stop_clock("Contacts");

   // DEBUG
   //for (auto i : CLp.v)
  //  printf("%d %g %X %X %d %d\n", ti/P.tdump, i.contactlength, i.ghost, i.ghostdir, i.i, i.j) ;

   //-------------------------------------------------------------------------------
   // Force and torque computation
   // DEBUG
   //static FILE * outdebug=fopen("Debug.txt", "w") ;

   Benchmark::start_clock("Forces");
   Tools::setzero(F) ; Tools::setzero(Fcorr) ; Tools::setzero(TorqueCorr) ;
   Tools::setgravity(F, P.g, P.m); // Actually set gravity is effectively also doing the setzero
   Tools::setzero(Torque);

   //Particle - particle contacts
   #pragma omp parallel default(none) shared(MP) shared(P) shared(X) shared(V) shared(Omega) shared(F) shared(Fcorr) shared(TorqueCorr) shared(Torque) shared(stdout)
   {
     int ID = omp_get_thread_num();
     ContactList & CLp = MP.CLp[ID] ; ContactList & CLw = MP.CLw[ID] ; Contacts & C =MP.C[ID] ;

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

      Tools::vAddFew(F[it->i], C.Act.Fn, C.Act.Ft, Fcorr[it->i]) ;
      Tools::vAddOne(Torque[it->i], C.Act.Torquei, TorqueCorr[it->i]) ;

      if (MP.ismine(ID,it->j))
      {
        Tools::vSubFew(F[it->j], C.Act.Fn, C.Act.Ft, Fcorr[it->j]) ;
        Tools::vAddOne(Torque[it->j], C.Act.Torquej, TorqueCorr[it->j]) ;
      }
      else
        MP.delaying(ID, it->j, C.Act) ;

      //Tools::vAdd(F[it->i], Act.Fn, Act.Ft) ; Tools::vSub(F[it->j], Act.Fn, Act.Ft) ; //F[it->i] += (Act.Fn + Act.Ft) ; F[it->j] -= (Act.Fn + Act.Ft) ;
      //Torque[it->i] += Act.Torquei ; Torque[it->j] += Act.Torquej ;
     }

     for (auto it = CLw.v.begin() ; it!=CLw.v.end() ; it++)
     {
      C.particle_wall(X[it->i],V[it->i],Omega[it->i],P.r[it->i], it->j/2, (it->j%2==0)?-1:1, *it) ;
      //Tools::vAdd(F[it->i], Act.Fn, Act.Ft) ; // F[it->i] += (Act.Fn+Act.Ft) ;
      //Torque[it->i] += Act.Torquei ;

      Tools::vAddFew(F[it->i], C.Act.Fn, C.Act.Ft, Fcorr[it->i]) ;
      Tools::vAddOne(Torque[it->i], C.Act.Torquei, TorqueCorr[it->i]) ;
     }
   }

   // Finish by sequencially adding the grains that were not owned by the parallel proc when computed
   for (int i=0 ; i<MP.P ; i++)
   {
     for (uint j=0 ; j<MP.delayed_size[i] ; j++)
     {
       Tools::vSubFew(F[MP.delayedj[i][j]], MP.delayed[i][j].Fn, MP.delayed[i][j].Ft, Fcorr[MP.delayedj[i][j]]) ;
       Tools::vAddOne(Torque[MP.delayedj[i][j]], MP.delayed[i][j].Torquej, TorqueCorr[MP.delayedj[i][j]]) ;
     }
   }
   MP.delayed_clean() ;

   Benchmark::stop_clock("Forces");

   //---------- Velocity Verlet step 3 : compute the new velocities
   Benchmark::start_clock("Verlet last");

   for (int i=0 ; i<N ; i++)
   {
    if (P.Frozen[i]) {Tools::setzero(TorqueOld[i]) ; Tools::setzero(F[i]) ; Tools::setzero(FOld[i]) ; Tools::setzero(V[i]) ; Tools::setzero(Omega[i]) ; }

    Tools::vAddScaled(V[i], dt/2./P.m[i], F[i], FOld[i]) ; //V[i] += (F[i] + FOld[i])*(dt/2./P.m[i]) ;
    Tools::vAddScaled(Omega[i], dt/2./P.I[i], Torque[i], TorqueOld[i]) ; // Omega[i] += (Torque[i]+TorqueOld[i])*(dt/2./P.I[i]) ;
    FOld[i]=F[i] ;
    TorqueOld[i]=Torque[i] ;
   }

   Benchmark::stop_clock("Verlet last");

   // Check events
   P.check_events(t, X,V,Omega) ;

   // Output something at some point I guess
   if (ti % P.tdump==0)
   {
    if (P.dumpkind==ExportType::CSV)
    {
        char path[500] ; sprintf(path, "%s/dump-%05d.csv", P.Directory.c_str(), ti) ;
        Tools::savecsv(path, X, P.r) ;
    }
    else if (P.dumpkind==ExportType::VTK)
    {
        char path[500] ; sprintf(path, "%s/dump-%05d.vtk", P.Directory.c_str(), ti) ;
        Tools::savevtk(path, P, X, {"", TensorType::NONE, &Omega}) ;
    }
    else if (P.dumpkind==ExportType::NETCDFF)
        printf("WARN: netcdf writing haven't been tested and therefore is not plugged in\n") ;
    else if (P.dumpkind==ExportType::XML)
    {
        P.xmlout->startTS(t);
        P.xmlout->writeArray("Position", &X, ArrayType::particles, EncodingType::ascii);
        P.xmlout->writeArray("Velocity", &V, ArrayType::particles, EncodingType::ascii);
        P.xmlout->stopTS();
    }
    else if (P.dumpkind==ExportType::XMLbase64)
    {
        P.xmlout->startTS(t);
        P.xmlout->writeArray("Position", &X, ArrayType::particles, EncodingType::base64);
        P.xmlout->writeArray("Velocity", &V, ArrayType::particles, EncodingType::base64);
        P.xmlout->stopTS();
    }
    else
      continue ;
   }
 }

//Tools::write1D ("Res.txt", TmpRes) ;
//Tools::writeinline_close() ;
Benchmark::write_all();
printf("This is the end ...\n") ;
return 0 ;
}
