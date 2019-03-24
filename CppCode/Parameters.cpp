#include "Parameters.h"
#include <boost/random.hpp>


int Parameters::set_boundaries()
{
    for (uint i=0 ; i<d ; i++)
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
void Parameters::perform_PBC (v1d & X, u_int32_t & PBCFlag)
{
 for (uint j=0 ; j<d ; j++)
 {
   if (Boundaries[j][3]!=0) continue ; // not a PBC
   if      (X[j]<Boundaries[j][0]) {X[j] += Boundaries[j][2] ; PBCFlag |= (1<<j) ;}
   else if (X[j]>Boundaries[j][1]) {X[j] -= Boundaries[j][2] ; PBCFlag |= (1<<j) ;}
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
return 0 ;
}
int Parameters::init_inertia ()
{
 for (int i=0; i<N ; i++)
 {
   I[i]=Tools::InertiaMomentum (r[i], rho) ;
 }
 return 0 ;
}
//---------------------------------------------------
void Parameters::load_datafile (char path[], v2d & X, v2d & V, v2d & Omega)
{
  ifstream in ;
  in.open(path) ;
  if (!in.is_open()) { printf("[Input] file cannot be open\n"); return ;}

  while (! in.eof())
  {
    interpret_command(in, X, V, Omega) ;
  }

  if ((dumpkind&ExportType::XML) || (dumpkind&ExportType::XMLbase64))
    xmlout= new XMLWriter(Directory+".xml") ;
}
//-------------------------------------------------
void Parameters::check_events(float time, v2d & X, v2d & V, v2d & Omega)
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
void Parameters::interpret_command (istream & in, v2d & X, v2d & V, v2d & Omega)
{
char line[5000] ; int id ;
std::vector <double> x (d,0) ; std::vector <double> omeg (d*(d-1)/2,0) ;

in>>line;
if (line[0]=='#') {in.getline(line, 5000) ; return ; } // The line is a comments

if (!strcmp(line,"event"))
{
  float time ;
  in >> time ;
  in.getline (line, 5000) ;
  //command.str(line) ;
  events.insert(make_pair(time,line)) ;
  printf("[INFO] Registering an event: %s\n", events.begin()->second.c_str()) ;
  return ;
}

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
  for (uint i=0 ; i<d ; i++) {in >> x[i] ; printf("%g ", x[i]) ; }
  X[id]=x ;
  printf("[INFO] Changing particle location.\n") ;
}
else if (!strcmp(line, "dimensions"))
{
  int nn; uint dd ; in>>dd ; in>>nn ;
  if (N!=nn || d!=dd) {printf("[ERROR] Dimension of number of particles not matching the input file requirements d=%d N=%d\n", d, N) ; std::exit(2) ; }
}
else if (!strcmp(line, "velocity"))
{
  in>>id ;
  for (uint i=0 ; i<d ; i++) in >> x[i] ;
  V[id]=x ;
  printf("[INFO] Changing particle velocity.\n") ;
}
else if (!strcmp(line, "omega"))
{
  in>>id ;
  for (uint i=0 ; i<d*(d-1)/2 ; i++) in >> omeg[i] ;
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
  for (uint i=0 ; i<d ; i++) in >> x[i] ;
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
 else if (!strcmp(line, "skin")) {in >> skin ; if (skin<r[0]) {skin=r[0] ; printf("The skin cannot be smaller than the radius") ; } skinsqr=skin*skin ; }
 else if (!strcmp(line, "dumpkind"))
 {
     string word ;
     in>>word ;
     if (word=="CSV") dumpkind |= ExportType::CSV ;
     else if (word=="VTK") dumpkind |= ExportType::VTK ;
     else if (word=="NETCDFF") dumpkind |= ExportType::NETCDFF ;
     else if (word=="XML") dumpkind |= ExportType::XML ;
     else if (word=="XMLbase64") dumpkind |= ExportType::XMLbase64 ;
     else if (word=="CSVA") dumpkind |= ExportType::CSVA ;
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
  else if (!strcmp(line, "rho"))
  {
    rho= m[0]/Tools::Volume(r[0]) ;
    printf("[Input] Using first particle mass to set rho: %g [M].[L]^-%d\n", rho, d) ;
  }
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
else if (!strcmp(line, "directory"))
{
  in>>Directory ;
  if (! experimental::filesystem::exists(line)) experimental::filesystem::create_directory(Directory);
}
else
    printf("[Input] Unknown command in input file |%s|\n", line) ;
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
    if (!strcmp(line, "square"))
    {
        auto m = *(std::max_element(r.begin(), r.end())) ;
        printf("%g\n", m) ;
        uint dd ;
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
    else if (!strcmp(line, "randomdrop"))
    {
        boost::random::mt19937 rng;
        boost::random::uniform_01<boost::mt19937> rand(rng) ;

        for (int i=0 ; i<N ; i++)
        {
         for(uint dd=0 ; dd < d ; dd++)
         {
             X[i][dd] = rand()*Boundaries[dd][2] + Boundaries[dd][0] ;

         }
        }

    }
    else if (!strcmp(line, "roughinclineplane"))
    {
      boost::random::mt19937 rng;
      boost::random::uniform_01<boost::mt19937> rand(rng) ;
      printf("Location::roughinclineplane assumes a plane of normal [1,0,0...] at location 0 along the 1st dimension.") ; fflush(stdout) ;
      auto m = *(std::max_element(r.begin(), r.end())) ; // Max radius
      double delta=0.1*m ;
      for (uint dd=0 ; dd<d ; dd++) X[0][dd]=Boundaries[dd][0]+m+delta ;
      Frozen[0]=true ;
      for (int i=1 ; i<N ; i++)
      {
        X[i]=X[i-1] ;
        for (uint dd=d-1 ; dd>=0 ; dd--)
        {
          X[i][dd] += 2*m+2*delta ;
          if (X[i][dd]>Boundaries[dd][1]-m-delta)
            X[i][dd] = Boundaries[dd][0]+m+delta ;
          else
            break ;
        }
        if (X[i][0]==Boundaries[0][0]+m+delta) Frozen[i]=true ;
        // randomize the previous grain
        for (uint dd=0 ; dd<d ; dd++)
          X[i-1][dd] += (rand()-0.5)*2*delta ;
      }
    }
    else if (!strcmp(line, "roughinclineplane2"))
    {
      boost::random::mt19937 rng;
      boost::random::uniform_01<boost::mt19937> rand(rng) ;
      printf("Location::roughinclineplane assumes a plane of normal [1,0,0...] at location 0 along the 1st dimension.") ; fflush(stdout) ;
      auto m = *(std::max_element(r.begin(), r.end())) ; // Max radius
      double delta=0.1*m ; int ddd ;
      for (uint dd=0 ; dd<d ; dd++) X[0][dd]=Boundaries[dd][0]+m ;
      Frozen[0]=true ; bool bottomlayer=true ;
      for (int i=1 ; i<N ; i++)
      {
        printf("%d \n",bottomlayer) ;
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
          for (uint dd=0 ; dd<d ; dd++)
            X[i-1][dd] += (rand()-0.5)*2*delta ;
        }
      }
    }
    else
        printf("ERR: no other initalisation location than square implemented\n") ;
}

//======================================
void Parameters::display_info(int tint, v2d& V, v2d& Omega, v2d& F, v2d& Torque, int nct, int ngst)
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

 double Vmax=Tools::norm(*max_element(V.begin(), V.end(), [](cv1d &a, cv1d &b) {return (Tools::normsq(a)<Tools::normsq(b)) ;})) ;
 double Omegamax=Tools::skewnorm(*max_element(Omega.begin(), Omega.end(), [](cv1d &a, cv1d &b) {return (Tools::skewnormsq(a)<Tools::skewnormsq(b)) ;})) ;
 double Torquemax=Tools::skewnorm(*max_element(Torque.begin(), Torque.end(), [](cv1d &a, cv1d &b) {return (Tools::skewnormsq(a)<Tools::skewnormsq(b)) ;})) ;
 double Fmax=Tools::norm(*max_element(F.begin(), F.end(), [](cv1d &a, cv1d &b) {return (Tools::normsq(a)<Tools::normsq(b)) ;})) ;

 printf("\n") ;
 printf("\e[80G Max V: %15g | V dt / R      = %15g \n", Vmax, Vmax*dt/Rmax) ;
 printf("\e[80G Max O: %15g | Omega dt      = %15g \n", Omegamax, Omegamax*dt) ;
 printf("\e[80G Max F: %15g | F dt dt / m r = %15g \n", Fmax, Fmax*dt*dt/(mmax*Rmax)) ;
 printf("\e[80G Max M: %15g | M dt / m R R  = %15g \n", Torquemax, Torquemax*dt/(mmax*Rmax*Rmax)) ;
 printf("\e[80G                        | g dt dt / R   = %15g \n", Tools::norm(g)*dt*dt/Rmax) ;

//printf("\e[u") ;
//printf("\\033%c", 0x38) ;
printf("\e[0G") ;
fflush(stdout) ;
}
if (first) first=false ;
}
//------------------------------------------
void Parameters::xml_header ()
{
   xmlout->openbranch("boundaries", {make_pair("length", to_string(d*2))}) ;
   for (uint i=0 ; i<d ; i++) xmlout->fic << Boundaries[i][0] << " " << Boundaries[i][1] << " " ;
   xmlout->closebranch() ;

   xmlout->openbranch("radius", {make_pair("length", to_string(N))}) ;
   for (auto v: r) xmlout->fic << v << " " ;
   xmlout->closebranch() ;
}




//-----------------------------------------
void Parameters::quit_cleanly()
{
  if ((dumpkind&ExportType::XML) || (dumpkind&ExportType::XMLbase64))
    xmlout->emergencyclose() ;
}
void Parameters::finalise()
{
  if ( (dumpkind&ExportType::XML) || (dumpkind&ExportType::XMLbase64))
    xmlout->close() ;
}
