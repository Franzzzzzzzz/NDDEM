#include "DEMND.h"
#include <signal.h>
#include "Benchmark.h"
long int cid=0 ; //DEBUG

int Tools::d=0 ;
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

 Contacts C(P) ; //Initialize the Contact class object
 ContactList CLp, CLg, CLw ;

 Action Act ;
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
   //TEST BEGIN
   //printf("%g \n", P.m[0]) ;
   //Tools::writeinline({X[0],V[0],Omega[0], X[1], V[1], Omega[1]}) ;
   //P.display_info(ti, V, Omega, F, Torque, CLp.v.size(), CLg.v.size()) ;
   //TEST END
   Benchmark::start_clock("Verlet 1st");
   for (int i=0 ; i<N ; i++)
   {
     //BEGIN TEST
     //Tools::setzero(FOld[0]) ;
     //V[0][0]=cos(t*2*M_PI)*(t<0.5?1:-1) ; V[0][2]=sin(t*2*M_PI) ; V[0][1]=0 ;
     //V[0][1]=0 ; V[0][0]=0.2-t ; V[0][2]=(0.5-t) ;
     //Omega[0][0]=0 ; Omega[0][1]=0 ; Omega[0][2]=-1 ;
     //END TEST
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

    //X[i] = X[i]+V[i]*dt + FOld[i] * (dt * dt / P.m[i] /2.) ;
    //A[i]= A[i] +
    //    Tools::matmult (Tools::skewexpand(Omega[i]), A[i]) * dt +
    //    Tools::matmult (Tools::skewexpand(TorqueOld[i])/P.I[i] + Tools::skewmatsquare(Omega[i]), A[i] ) * (dt * dt /2.) ;

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
   cp tmpcp(0,0,d,0,nullptr) ; double sum=0, sum2 ;
   CLp.reset() ; CLg.reset() ; CLw.reset();

   for (int i=0 ; i<N ; i++)
   {
       Benchmark::start_clock("Contacts_part");
       tmpcp.setinfo(CLp.default_action());


       for (int j=i+1 ; j<N ; j++) // Regular particles
       {
           sum=0 ;
           for (int k=0 ; k<d ; k++) sum+= (X[i][k]-X[j][k])*(X[i][k]-X[j][k]) ;
           if (sum<(P.r[i]+P.r[j])*(P.r[i]+P.r[j]))
           {
               tmpcp.i=i ; tmpcp.j=j ; tmpcp.contactlength=sqrt(sum) ; tmpcp.isghost=0 ;
               CLp.insert(tmpcp) ;

               //if (CLp.cid>345090 && CLp.cid<345110 && i==0 && j==1) printf("#n %d %d %g %g\n", i, j, X[i][0], X[j][0]) ;
           }

           // Test with ghosts
           if (Ghost[j]) //j has at least 1 ghost
           {
               u_int32_t gst = Ghost[j], gst_dir=Ghost_dir[j] ;
               for (u_int8_t n=0 ; gst ; gst>>=1, gst_dir>>=1, n++ )
               {
                   if (gst&1)
                   {
                       double Delta= (gst_dir&1?-1:1) * P.Boundaries[n][2] ;
                       sum2= sum + Delta*(2*(X[j][n]-X[i][n]) + Delta) ;
                       if (sum2<(P.r[i]+P.r[j])*(P.r[i]+P.r[j]))
                       {
                        tmpcp.i=i ; tmpcp.j=j ; tmpcp.contactlength=sqrt(sum2) ; tmpcp.isghost = (n+1) * (gst_dir&1?-1:1) ;
                        CLp.insert(tmpcp) ;
                       }
                   }
               }
           }
       }
       Benchmark::stop_clock("Contacts_part");

       Benchmark::start_clock("Contacts_bound");
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
       Benchmark::stop_clock("Contacts_bound");
   }
   CLp.finalise() ;
   CLg.finalise() ;
   CLw.finalise() ;
   Benchmark::stop_clock("Contacts");

   //-------------------------------------------------------------------------------
   // Force and torque computation
   // DEBUG
   //static FILE * outdebug=fopen("Debug.txt", "w") ;

   Benchmark::start_clock("Forces");
   Tools::setzero(F) ; Tools::setzero(Fcorr) ; Tools::setzero(TorqueCorr) ;
   v1d tmp, previous ;
   Tools::setgravity(F, P.g, P.m); // Actually set gravity is effectively also doing the setzero
   Tools::setzero(Torque);

   //Particle - particle contacts
   for (auto it = CLp.v.begin() ; it!=CLp.v.end() ; it++)
   {
    if (it->isghost==0)
    {
        Act = C.particle_particle(X[it->i], V[it->i], Omega[it->i], P.r[it->i],
                              X[it->j], V[it->j], Omega[it->j], P.r[it->j], *it) ; cid++ ;
    }
    else
    {
        Act=C.particle_ghost(X[it->i], V[it->i], Omega[it->i], P.r[it->i],
                             X[it->j], V[it->j], Omega[it->j], P.r[it->j], *it) ; cid++ ;
    }

    Tools::vAddFew(F[it->i], Act.Fn, Act.Ft, Fcorr[it->i]) ;
    Tools::vSubFew(F[it->j], Act.Fn, Act.Ft, Fcorr[it->j]) ;
    Tools::vAddOne(Torque[it->i], Act.Torquei, TorqueCorr[it->i]) ;
    Tools::vAddOne(Torque[it->j], Act.Torquej, TorqueCorr[it->j]) ;

/*    tmp=(Act.Fn+Act.Ft)-Fcorr[it->i] ;
    previous=F[it->i] ;
    F[it->i] += tmp ;
    Fcorr[it->i] = F[it->i]-previous-tmp ;

    tmp=(-Act.Fn-Act.Ft)-Fcorr[it->j] ;
    previous=F[it->j] ;
    F[it->j] += tmp ;
    Fcorr[it->j] = F[it->j]-previous-tmp ;

    tmp=(Act.Torquei)-TorqueCorr[it->i] ;
    previous=Torque[it->i] ;
    Torque[it->i] += tmp ;
    TorqueCorr[it->i] = Torque[it->i]-previous-tmp ;

    tmp=(Act.Torquej)-TorqueCorr[it->j] ;
    previous=Torque[it->j] ;
    Torque[it->j] += tmp ;
    TorqueCorr[it->j] = Torque[it->j]-previous-tmp ; */

    //Tools::vAdd(F[it->i], Act.Fn, Act.Ft) ; Tools::vSub(F[it->j], Act.Fn, Act.Ft) ; //F[it->i] += (Act.Fn + Act.Ft) ; F[it->j] -= (Act.Fn + Act.Ft) ;
    //Torque[it->i] += Act.Torquei ; Torque[it->j] += Act.Torquej ;
   }

   for (auto it = CLw.v.begin() ; it!=CLw.v.end() ; it++)
   {
    Act=C.particle_wall(X[it->i],V[it->i],Omega[it->i],P.r[it->i], it->j/2, (it->j%2==0)?-1:1, *it) ;
    //Tools::vAdd(F[it->i], Act.Fn, Act.Ft) ; // F[it->i] += (Act.Fn+Act.Ft) ;
    //Torque[it->i] += Act.Torquei ;

    /*tmp=(Act.Fn+Act.Ft)-Fcorr[it->i] ;
    previous=F[it->i] ;
    F[it->i] += tmp ;
    Fcorr[it->i] = F[it->i]-previous-tmp ;

    tmp=(Act.Torquei)-TorqueCorr[it->i] ;
    previous=Torque[it->i] ;
    Torque[it->i] += tmp ;
    TorqueCorr[it->i] = Torque[it->i]-previous-tmp ;*/
    Tools::vAddFew(F[it->i], Act.Fn, Act.Ft, Fcorr[it->i]) ;
    Tools::vAddOne(Torque[it->i], Act.Torquei, TorqueCorr[it->i]) ;
   }

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
    //if (ti > 388800 && ti<389000)  fprintf(outdebug, "{%d %g %g|%g}",i, X[i][0], X[i][2], Omega[i][1] );
   }
   C.clean_history() ; //Contact history cleaning and preparation for the next iteration

   Benchmark::stop_clock("Verlet last");


   // Output something at some point I guess
   if (ti % P.tdump==0)
   {
    if (P.dumpkind==ExportType::CSV)
    {
        char path[500] ; sprintf(path, "Output/dump-%05d.csv", ti) ;
        Tools::savecsv(path, X, P.r) ;
    }
    else if (P.dumpkind==ExportType::VTK)
    {
        char path[500] ; sprintf(path, "Output/dump-%05d.vtk", ti) ;
        Tools::savevtk(path, X, {"Omega", TensorType::SKEWTENSOR, &Omega}) ;
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
