#include "Parameters.h"
#include <boost/random.hpp>


int Parameters::set_boundaries()
{
    for (int i=0 ; i<d ; i++)
    {
        Boundaries[i][0]= 0 ; 
        Boundaries[i][1]= 1 ;
        Boundaries[i][2]=Boundaries[i][1]-Boundaries[i][0] ; //Precomputed to increase speed
        Boundaries[i][3]=0 ; // PBC by default
    }
    //np.savetxt('Boundaries.csv', Boundaries , delimiter=',', fmt='%.6g', header='Low,High,Size,Type', comments='')
    return 0 ; 
}
//----------------------------------------------------
void Parameters::perform_PBC(v1d & X)
{
 for (int j=0 ; j<d ; j++)
 {
   if (Boundaries[j][3]!=0) continue ; // not a PBC      
   if      (X[j]<Boundaries[j][0]) X[j] += Boundaries[j][2] ; 
   else if (X[j]>Boundaries[j][1]) X[j] -= Boundaries[j][2] ; 
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
int Parameters::init_mass ()
{
 for (int i=0 ; i<N ; i++)
   m[i]=rho*Tools::Volume(r[i]) ;  
}
int Parameters::init_inertia ()
{
 for (int i=0; i<N ; i++)
 {
   I[i]=Tools::InertiaMomentum (r[i], rho) ;
 }
}
//---------------------------------------------------
void Parameters::load_datafile (char path[], v2d & X, v2d & V, v2d & Omega)
{
  ifstream in ; 
  char line[5000] ; int id ; 
  in.open(path) ; std::vector <double> x (d,0) ; std::vector <double> omeg (d*(d-1)/2,0) ;
  if (!in.is_open()) { printf("[Input] file cannot be open\n"); return ;}

  while (! in.eof())
  {
      in>>line ;
      if (line[0]=='#') {in.getline(line, 5000) ; continue ; } // The line is a comments
      if (!strcmp(line, "boundary"))
      {
        in>>id ; 
        in>>line ; 
        if (!strcmp(line, "PBC")) Boundaries[id][3]=0 ; 
        else if (!strcmp(line, "WALL")) Boundaries[id][3]=1 ;
        else printf("[Input] Unknown boundary condition, unchanged.\n") ; 
        in >> Boundaries[id][0] ; in>> Boundaries[id][1] ; 
        Boundaries[id][2]=Boundaries[id][1]-Boundaries[id][0] ; 
       printf("[INFO] Changing BC.\n") ; 
      }
      else if (!strcmp(line, "location"))
      {
        in>>id ;
        for (int i=0 ; i<d ; i++) {in >> x[i] ; printf("%g ", x[i]) ; }
        X[id]=x ; 
        printf("[INFO] Changing particle location.\n") ; 
      }
      else if (!strcmp(line, "dimensions"))
      {
        int nn, dd ; in>>dd ; in>>nn ; 
        if (N!=nn || d!=dd) {printf("[ERROR] Dimension of number of particles not matching the input file requirements d=%d N=%d\n", d, N) ; std::exit(2) ; }
      }
      else if (!strcmp(line, "velocity"))
      {
        in>>id ;
        for (int i=0 ; i<d ; i++) in >> x[i] ;
        V[id]=x ; 
        printf("[INFO] Changing particle velocity.\n") ; 
      }
      else if (!strcmp(line, "omega"))
      {
        in>>id ;
        for (int i=0 ; i<d*(d-1)/2 ; i++) in >> omeg[i] ;
        Omega[id]=omeg ;
        printf("[INFO] Changing particle angular velocity.\n") ; 
      }
      else if (!strcmp(line, "freeze"))
      {
        in>>id ;
        Frozen[id]=true ; 
        printf("[INFO] Freezing particle.\n") ; 
      }
      else if (!strcmp(line, "radius"))
      {
        in>>id ; double radius ; in>>radius ; 
        if (id==-1) for (int i=0 ; i<N ; i++) r[i]=radius ; 
        else r[id]=radius ;
        printf("[INFO] Set radius of particle.\n") ; 
      }
      else if (!strcmp(line, "mass"))
      {
        in>>id ; double mass ; in>>mass ; 
        if (id==-1) for (int i=0 ; i<N ; i++) m[i]=mass ; 
        else m[id]=mass ;
        printf("[INFO] Set mass of particle.\n") ; 
      }
      else if (!strcmp(line, "gravity"))
      {
        for (int i=0 ; i<d ; i++) in >> x[i] ;
        g=x ; 
        printf("[INFO] Changing gravity.\n") ; 
      }
      else if (!strcmp(line, "set"))
      {
       in>>line ; 
       if (!strcmp(line, "Kn")) in>>Kn ; 
       else if (!strcmp(line, "Kt")) in>>Kt ; 
       else if (!strcmp(line, "GammaN")) in>>Gamman ; 
       else if (!strcmp(line, "GammaT")) in>>Gammat ; 
       else if (!strcmp(line, "rho")) in>>rho ; 
       else if (!strcmp(line, "Mu")) in>>Mu ; 
       else if (!strcmp(line, "T")) in>>T ; 
       else if (!strcmp(line, "tdump")) in>>tdump ; 
       else if (!strcmp(line, "dumpkind"))
       {
           string word ; 
           in>>word ; 
           if (word=="CSV") dumpkind=ExportType::CSV ; 
           else if (word=="VTK") dumpkind=ExportType::VTK ; 
           else if (word=="NETCDFF") dumpkind=ExportType::NETCDFF ; 
           else if (word=="XML") dumpkind=ExportType::XML ; 
           else if (word=="XMLbase64") dumpkind=ExportType::XMLbase64 ; 
       } 
       else if (!strcmp(line, "tinfo")) in>>tinfo ; 
       else if (!strcmp(line, "dt")) in>>dt ; 
       else printf("[Input] Unknown parameter to set\n") ;
       
       printf("[INFO] Setting a parameter.\n") ; 
      }
      else if (!strcmp(line, "auto"))
      {
        in>>line ;
        if (!strcmp(line, "mass")) init_mass() ; 
        else if (!strcmp(line, "inertia")) init_inertia() ; 
        else if (!strcmp(line, "location")) 
        {
            in >> line ; 
            init_locations(line, X) ;
            printf("[Input] Set all particle locations\n") ; 
        }
        else printf("[WARN] Unknown auto command in input script\n") ; 
        printf("[Input] Doing an auto \n") ; 
      }
      else
          printf("[Input] Unknown command in input file |%s|\n", line) ; 
  }
  
}



//=====================================
void Parameters::remove_particle (int idx, v2d & X, v2d & V, v2d & A, v2d & Omega, v2d & F, v2d & FOld, v2d & Torque, v2d & TorqueOld)
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
void Parameters::add_particle (v2d & X, v2d & V, v2d & A, v2d & Omega, v2d & F, v2d & FOld, v2d & Torque, v2d & TorqueOld)
{
    printf("add_particle NOT IMPLEMENTED YET\n") ; 
}
void Parameters::init_locations (char *line, v2d & X)
{
    if (strcmp(line, "square")) {printf("ERR: no other initalisation locaiton than square implemented\n") ; return ; }
    
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
        if (dd==d) {printf("WARN: cannot affect all particles on the square lattice, not enoush space in the simulation box\n") ; break ; }
    }
}

//======================================
void Parameters::display_info(int tint, v2d& V, v2d& Omega, v2d& F, v2d& Torque, int nct, int ngst)
{
static bool first=true ; 
static char letters[]={'|', '/', '-', '\\'} ;
static double Rmax, mmax ; 
if (first)
{
 Rmax=*max_element(r.begin(), r.end()) ; 
 mmax=*max_element(m.begin(), m.end()) ; 
 first=false ; 
 printf("\e[10S\e[9A") ;
}

printf("\e[s%c\n", letters[tint%4]) ; 
if (tint%tinfo==0)
{
 for (int i=0 ; i<(tint*100)/int(T/dt) ; i++) printf("#") ;    
  //printf("%d %d %d ",tint, T, (tint*100)/T) ; 
 printf("\n NContacts: %d | Nghosts: %d \n", nct, ngst) ; 

 double Vmax=Tools::norm(*max_element(V.begin(), V.end(), [](cv1d &a, cv1d &b) {return (Tools::normsq(a)<Tools::normsq(b)) ;})) ; 
 double Omegamax=Tools::skewnorm(*max_element(Omega.begin(), Omega.end(), [](cv1d &a, cv1d &b) {return (Tools::skewnormsq(a)<Tools::skewnormsq(b)) ;})) ; 
 double Torquemax=Tools::skewnorm(*max_element(Torque.begin(), Torque.end(), [](cv1d &a, cv1d &b) {return (Tools::skewnormsq(a)<Tools::skewnormsq(b)) ;})) ; 
 double Fmax=Tools::norm(*max_element(F.begin(), F.end(), [](cv1d &a, cv1d &b) {return (Tools::normsq(a)<Tools::normsq(b)) ;})) ; 
 
 printf("\n") ; 
 printf("Max V: %10g | V dt / R      = %10g \n", Vmax, Vmax*dt/Rmax) ;
 printf("Max O: %10g | Omega dt      = %10g \n", Omegamax, Omegamax*dt) ;
 printf("Max F: %10g | F dt dt / m r = %10g \n", Fmax, Fmax*dt*dt/(mmax*Rmax)) ;
 printf("Max M: %10g | M dt / m R R  = %10g \n", Torquemax, Torquemax*dt/(mmax*Rmax*Rmax)) ;
 printf("            | g dt dt / R   = %10g \n", Tools::norm(g)*dt*dt/Rmax) ;
}
    
printf("\e[u") ; 
fflush(stdout) ; 
}





