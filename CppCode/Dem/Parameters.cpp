#include "Parameters.h"
#include <boost/random.hpp>


int Parameters::set_boundaries()
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
void Parameters::perform_PBC (v1d & X, u_int32_t & PBCFlag)
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
void Parameters::perform_MOVINGWALL ()
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

  for (auto v : dumps)
    if (v.first==ExportType::XML || v.first==ExportType::XMLbase64)
      xmlout= new XMLWriter(Directory+"/dump.xml") ;

  in.close() ;
  // Self copy :)
  experimental::filesystem::path p (path) ;
  experimental::filesystem::path pcp (Directory+"/in") ;// pcp/= p.filename() ;
  copy_file(p,pcp,experimental::filesystem::copy_options::overwrite_existing);
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
  if (!strcmp(line, "PBC")) Boundaries[id][3]=static_cast<int>(WallType::PBC) ;
  else if (!strcmp(line, "WALL")) Boundaries[id][3]=static_cast<int>(WallType::WALL) ;
  else if (!strcmp(line, "MOVINGWALL")) {Boundaries[id][3]=static_cast<int>(WallType::MOVINGWALL) ; Boundaries[id].resize(4+2, 0) ; }
  else printf("[Input] Unknown boundary condition, unchanged.\n") ;
  in >> Boundaries[id][0] ; in>> Boundaries[id][1] ;
  Boundaries[id][2]=Boundaries[id][1]-Boundaries[id][0] ;
  if (Boundaries[id][3]==static_cast<int>(WallType::MOVINGWALL))
  {in >> Boundaries[id][4] ; in >> Boundaries[id][5] ; }
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
  int nn; int dd ; in>>dd ; in>>nn ;
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
else if (!strcmp(line, "gravityangle"))
{
  double intensity, angle ;
  in >> intensity >> angle ;
  Tools::setzero(x) ;
  x[0] = -intensity * cos(angle / 180. * M_PI) ;
  x[1] = intensity * sin(angle / 180. * M_PI) ;
  g=x ;
  //for (auto v: g) printf("%g ", v) ;
  printf("[INFO] Changing gravity angle in degree between x0 and x1.\n") ;
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
 else if (!strcmp(line, "orientationtracking")) in >> orientationtracking ;
 else if (!strcmp(line, "skin")) {in >> skin ; if (skin<r[0]) {skin=r[0] ; printf("The skin cannot be smaller than the radius") ; } skinsqr=skin*skin ; }
 else if (!strcmp(line, "dumps"))
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
     else if (word =="Orientation") dumplist |= ExportData::ORIENTATION ;
     else if (word =="Coordination") dumplist |= ExportData::COORDINATION ;
     else printf("Unknown asked data %s\n", word.c_str()) ;
   }

   dumps.push_back(make_pair(dumpkind,dumplist)) ;
   }
   LABEL_leave: ; // Goto label (I know, not beautiful, but makes sense here really)
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
void Parameters::add_particle (/*v2d & X, v2d & V, v2d & A, v2d & Omega, v2d & F, v2d & FOld, v2d & Torque, v2d & TorqueOld*/)
{
    printf("add_particle NOT IMPLEMENTED YET\n") ;
}
void Parameters::init_locations (char *line, v2d & X)
{
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
    else if (!strcmp(line, "randomdrop"))
    {
        boost::random::mt19937 rng;
        boost::random::uniform_01<boost::mt19937> rand(rng) ;

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
    else if (!strcmp(line, "roughinclineplane"))
    {
      boost::random::mt19937 rng;
      boost::random::uniform_01<boost::mt19937> rand(rng) ;
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
      boost::random::mt19937 rng;
      boost::random::uniform_01<boost::mt19937> rand(rng) ;
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
   for (int i=0 ; i<d ; i++) xmlout->fic << Boundaries[i][0] << " " << Boundaries[i][1] << " " ;
   xmlout->closebranch() ;

   xmlout->openbranch("radius", {make_pair("length", to_string(N))}) ;
   for (auto v: r) xmlout->fic << v << " " ;
   xmlout->closebranch() ;
}

//-----------------------------------------
void Parameters::quit_cleanly()
{
  for (auto v : dumps)
    if ((v.first == ExportType::XML) || (v.first == ExportType::XMLbase64))
      xmlout->emergencyclose() ;
}
void Parameters::finalise()
{
  for (auto v : dumps)
    if ((v.first == ExportType::XML) || (v.first == ExportType::XMLbase64))
      xmlout->close() ;
}


//========================================
int Parameters::dumphandling (int ti, double t, v2d &X, v2d &V, v1d &Vmag, v2d &A, v2d &Omega, v1d &OmegaMag, vector<u_int32_t> &PBCFlags, v1d & Z)
{
  static bool xmlstarted=false ;

  for (auto v : dumps)
  {
    if (v.first==ExportType::CSV)
    {
      char path[500] ; sprintf(path, "%s/dump-%05d.csv", Directory.c_str(), ti) ;
      Tools::norm(Vmag, V) ; Tools::norm(OmegaMag, Omega) ;
      Tools::savecsv(path, X, r, PBCFlags, Vmag, OmegaMag, Z) ; //These are always written for CSV, independent of the dumplist
      if (v.second & ExportData::ORIENTATION)
      {
        char path[500] ; sprintf(path, "%s/dumpA-%05d.csv", Directory.c_str(), ti) ;
        Tools::savecsv(path, A) ;
      }
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
        if (v.second & ExportData::COORDINATION) {tmp.push_back(Z) ; val.push_back({"Coordination", TensorType::SCALAR, &tmp}) ;  }
        Tools::savevtk(path, *this, X, val) ;
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
