#include "DEMND.h"
#include <signal.h>
#include "Benchmark.h"

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
 assert(P.d<sizeof(int)) ; 
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
 vector < vector <int> > Ghosts ; 
 vector < double > Ghosts_deltas ; 
 int Nghosts=0 ; 
 v1d Atmp (d*d, 0) ; 

 vector <int> tmpghost (2,0) ; 
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
   P.display_info(ti, V, Omega, F, Torque, CLp.v.size(), CLg.v.size()) ; 
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
    for (int j=0 ; j<d ; j++)
    {
     if (P.Boundaries[j][3] != 0) continue ; 
     if      (X[i][j] < P.Boundaries[j][0] + 2*P.r[i]) { tmpghost[0]=i ; tmpghost[1]=j ; Ghosts.push_back(tmpghost) ; Ghosts_deltas.push_back( P.Boundaries[j][2]) ; }
     else if (X[i][j] > P.Boundaries[j][1] - 2*P.r[i]) { tmpghost[0]=i ; tmpghost[1]=j ; Ghosts.push_back(tmpghost) ; Ghosts_deltas.push_back(-P.Boundaries[j][2]) ; }
    }
    Nghosts=Ghosts.size() ; 
   }
   Benchmark::stop_clock("Verlet 1st");
   
   //---------- Velocity Verlet step 2 : compute the forces and torques
   // Contact detection (sequential, can be paralelised easily with openmp)
   
   Benchmark::start_clock("Contacts");
   cp tmpcp(0,0,d,0,nullptr) ; 
   CLp.reset() ; CLg.reset() ; CLw.reset();
   for (int i=0 ; i<N ; i++) 
   {
       Benchmark::start_clock("Contacts_part");
       tmpcp.setinfo(CLp.default_action());
       for (int j=i+1 ; j<N ; j++) // Regular particles
       {
           double sum=0 ; 
           for (int k=0 ; k<d ; k++) sum+= (X[i][k]-X[j][k])*(X[i][k]-X[j][k]) ; 
           if (sum<(P.r[i]+P.r[j])*(P.r[i]+P.r[j]))
           {
               tmpcp.i=i ; tmpcp.j=j ; tmpcp.contactlength=sqrt(sum) ; 
               CLp.insert(tmpcp) ; 
           }  
       }
       Benchmark::stop_clock("Contacts_part");
       
       Benchmark::start_clock("Contacts_ghost"); 
       //printf("%d ", Nghosts) ; 
       tmpcp.setinfo(CLg.default_action());
       double sum=0, save ; v1d * ghst ; 
       for (int j=0; j<Nghosts ; j++) // Ghost particles
       {
           if (i==Ghosts[j][0]) continue ; 
           ghst= &(X[Ghosts[j][0]]) ; 
           save=(*ghst)[Ghosts[j][1]] ;
           //Xg=X[Ghosts[j][0]]+Tools::unitvec(Ghosts[j][1])*Ghosts_deltas[j] ; 
           (*ghst)[Ghosts[j][1]] += Ghosts_deltas[j] ;
           for (int k=0 ; k<d ; k++) sum+= (X[i][k]-(*ghst)[k])*(X[i][k]-(*ghst)[k]) ;  
           if (sum<(P.r[i]+P.r[Ghosts[j][0]])*(P.r[i]+P.r[Ghosts[j][0]]))
           {
               tmpcp.i=i ; tmpcp.j=Ghosts[j][0] ; tmpcp.contactlength=sqrt(sum) ; 
               CLg.insert(tmpcp) ; 
           }
           (*ghst)[Ghosts[j][1]] = save ;
       }
       Benchmark::stop_clock("Contacts_ghost");
       
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
   
   Benchmark::start_clock("Forces");
   Tools::setgravity(F, P.g, P.m); Tools::setzero(Torque);
   
   //Particle - particle contacts
   for (auto it = CLp.v.begin() ; it!=CLp.v.end() ; it++)
   {
    Act = C.particle_particle(X[it->i], V[it->i], Omega[it->i], P.r[it->i], 
                              X[it->j], V[it->j], Omega[it->j], P.r[it->j], *it) ;
    Tools::vAdd(F[it->i], Act.Fn, Act.Ft) ; Tools::vSub(F[it->j], Act.Fn, Act.Ft) ; //F[it->i] += (Act.Fn + Act.Ft) ; F[it->j] -= (Act.Fn + Act.Ft) ;
    Torque[it->i] += Act.Torquei ; Torque[it->j] += Act.Torquej ; 
    //if (isdumptime) {it->setinfo(Act);}
   }
   
   for (auto it = CLg.v.begin() ; it!=CLg.v.end() ; it++)
   {
    Act = C.particle_ghost (X[it->i], V[it->i], Omega[it->i], P.r[it->i], 
                            X[Ghosts[it->j][0]]+Tools::unitvec(Ghosts[it->j][1])*Ghosts_deltas[it->j], V[Ghosts[it->j][0]], Omega[Ghosts[it->j][0]], P.r[Ghosts[it->j][0]], *it) ;   
    Tools::vAdd(F[it->i], Act.Fn, Act.Ft) ; //F[it->i] += (Act.Fn + Act.Ft) ;
    Torque[it->i] += Act.Torquei ;
    //if (isdumptime) {it->setinfo(Act);}
   }
   
   for (auto it = CLw.v.begin() ; it!=CLw.v.end() ; it++)
   {
    Act=C.particle_wall(X[it->i],V[it->i],Omega[it->i],P.r[it->i], it->j/2, (it->j%2==0)?-1:1, *it) ; 
    Tools::vAdd(F[it->i], Act.Fn, Act.Ft) ; // F[it->i] += (Act.Fn+Act.Ft) ; 
    Torque[it->i] += Act.Torquei ;
    //if (isdumptime) {it->setinfo(Act);}
   }
   
   
   Benchmark::stop_clock("Forces");
   
   /* Force computation superseeded by the contact lists ...
   Tools::setzero(F) ; Tools::setzero(Torque) ; 
   for (int i=0 ; i<N ; i++)
   {
    F[i]+= P.g * P.m[i] ; 
    
    //-----Particle - particle contacts
    for (int j=i+1 ; j<N ; j++)
    {
      Act=C.particle_particle(X[i], V[i], Omega[i], P.r[i], i, X[j], V[j], Omega[j], P.r[j], j) ; 
      F[i] += (Act.Fn + Act.Ft) ; 
      F[j] -= (Act.Fn + Act.Ft) ;
      Torque[i] += Act.Torquei ; 
      Torque[j] += Act.Torquej ; 
    }
    
    //-----Particle - ghosts contacts
    for (int j=0 ; j<Nghosts ; j++)
    {
      
      Act=C.particle_ghost (X[i], V[i], Omega[i], P.r[i], i, 
            X[Ghosts[j][0]]+Tools::unitvec(Ghosts[j][1])*Ghosts_deltas[j], V[Ghosts[j][0]], Omega[Ghosts[j][0]], P.r[Ghosts[j][0]], N+Ghosts[j][0]) ;   
      F[i] += (Act.Fn + Act.Ft) ;
      Torque[i] += Act.Torquei ;
      
    }

    //---------Particle - Walls contacts
    for (int j=0 ; j<d ; j++)
    {
      if (P.Boundaries[j][3]!=1) continue ; 
      Act=C.particle_wall(X[i],V[i],Omega[i],P.r[i],i,j,0,-1,P.Boundaries[j][0]) ;
      F[i] += (Act.Fn+Act.Ft) ; 
      Torque[i] += Act.Torquei ;
      Act=C.particle_wall(X[i],V[i],Omega[i],P.r[i],i,j,1, 1,P.Boundaries[j][1]) ;
      
      F[i] += (Act.Fn+Act.Ft) ; 
      Torque[i] += Act.Torquei ;
    } 
   }*/
   
   
   //---------- Velocity Verlet step 3 : compute the new velocities
   Benchmark::start_clock("Verlet last");
   Ghosts.clear() ; Ghosts_deltas.clear() ; 
      
   for (int i=0 ; i<N ; i++)
   { 
    if (P.Frozen[i]) {Tools::setzero(TorqueOld[i]) ; Tools::setzero(F[i]) ; Tools::setzero(FOld[i]) ; Tools::setzero(V[i]) ; Tools::setzero(Omega[i]) ; }
     
    Tools::vAddScaled(V[i], dt/2./P.m[i], F[i], FOld[i]) ; //V[i] += (F[i] + FOld[i])*(dt/2./P.m[i]) ; 
    Tools::vAddScaled(Omega[i], dt/2./P.I[i], Torque[i], TorqueOld[i]) ; // Omega[i] += (Torque[i]+TorqueOld[i])*(dt/2./P.I[i]) ; 
    FOld[i]=F[i] ; 
    TorqueOld[i]=Torque[i] ; 
   }
   
   Nghosts=Ghosts.size() ; 
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
        Tools::savevtk(path, X) ; 
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