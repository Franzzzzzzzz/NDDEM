#ifndef PARAMETERS_H
#define PARAMETERS_H

//using json = nlohmann::json;
using json = nddem::json ; 

enum FileFormat {NDDEM, Liggghts, MercuryData, MercuryVTU, Yade, Interactive} ;
enum FileType {both, particles, contacts} ;
//enum DataValue {radius, mass, Imom, pos, vel, omega, id1, id2, pospq, fpq, mpq, mqp} ;

class Param {
public:

  int skipT=0 ;
  int maxT =-1;
  vector <string> flags = {} ;
  int dim=-1 ;
  vector <int> boxes ;
  vector <vector <double> > boundaries ;
  string save="" ;
  vector<string> saveformat ;
  double default_density=-1, default_radius=-1 ;
  std::vector<double> default_sqaxes{-1.,-1.,-1.}, default_sqpower{2,2,2} ; 

  AverageType timeaverage = AverageType::None ;

  Windows window = Windows::Lucy3D ;
  double windowsize ;
  vector<bool> periodicity ;
  vector<double>delta ;

  std::map <std::string, bool> requiredfieldset = {{"dimension", false}, {"window size", false}, {"fields", false}, {"max time", false}, {"boundaries", false}, {"boxes", false}} ;

  //----- Files
  struct File {
      FileFormat fileformat;
      FileType filetype ;
      std::map <std::string, std::string> mapping ;
      Reader * reader   ;
      //~File() {if (reader != nullptr) {reader->close() ; delete (reader) ; }}
  } ;
  std::vector <File> files ;

  //----- Extra fields
  struct ExtaField {
    string name  ;
    TensorOrder order ;
    FieldType type ;
    std::optional<std::string> mapping ; 
    int datalocation ; 
  } ;
  vector<ExtaField> extrafields ;

  int curts=0 ;
  bool tsread = false ;

  void from_json (json & j) ;
  int read_timestep (int ts, bool particleonly=false) ;
  int set_data (Data & cgdata) ;
  int get_num_particles () ;
  int get_num_contacts () ;
  double get_volume() ;
  double * get_data(DataValue datavalue, int dd=0, std::string name="") ;
  void post_init () ;


private :
  int identify_max_level() ;
  Windows identify_window(std::string windowname) ;
  void process_extrafields (json &j) ;
  void process_file (json &j) ;
} ;

//=================================================================================================
void Param::from_json(json &j)
{
    for (auto v : j.items())
    {
             if (v.key() == "file") {process_file(v.value()) ; }
        else if (v.key() == "savefile") save = v.value().get<string>() ;
        else if (v.key() == "saveformat")
        {
            try {saveformat = v.value().get<vector<string>>();}
            catch(...) {string a = v.value().get<string>(); saveformat.push_back(a) ; }
        }
        else if (v.key() == "window size")
        { windowsize = v.value().get<double>(); requiredfieldset[v.key()] = true ; }
        else if (v.key() == "skip") skipT = v.value().get<int>() ;
        else if (v.key() == "max time") { maxT = v.value().get<int>() ; requiredfieldset[v.key()] = true ;}
        else if (v.key() == "fields") { flags = v.value().get<vector<string>>(); requiredfieldset[v.key()] = true ; }
        else if (v.key() == "boxes")  {boxes = v.value().get<decltype(boxes)>(); requiredfieldset[v.key()] = true ; }
        else if (v.key() == "boundaries") {boundaries = v.value().get<decltype(boundaries)>(); requiredfieldset[v.key()] = true ; }
        else if (v.key() == "time average")
        {
            if (v.value().get<string>() == "None") timeaverage = AverageType::None ;
            else if (v.value().get<string>() == "Final") timeaverage = AverageType::Final ;
            else if (v.value().get<string>() == "Intermediate") timeaverage = AverageType::Intermediate ;
            else if (v.value().get<string>() == "Intermediate and Final" || v.value().get<string>() =="Final and Intermediate") timeaverage = AverageType::Both ;
            else if (v.value().get<string>() == "Pre pass 5") timeaverage = AverageType::Pre5 ;
            else if (v.value().get<string>() == "Intermediate and pre pass 5") timeaverage = AverageType::IntermediateAndPre5 ;
            else
                printf("WARN: unknown average type (allowed: 'None', 'Final', 'Intermediate', 'Intermediate and Final', 'Pre pass 5', 'Intermediate and pre pass 5'). Case sensitive\n") ;
        }
        else if (v.key() == "window") {window = identify_window(v.value().get<string>()); }
        else if (v.key() == "periodicity") {periodicity = v.value().get<decltype(periodicity)>();}
        else if (v.key() == "density") { default_density=v.value().get<double>() ; }
        else if (v.key() == "radius") { default_radius=v.value().get<double>() ; }
        else if (v.key() == "diameter") { default_radius=v.value().get<double>()/2. ; }
        else if (v.key() == "superquadric") {
            default_sqaxes = v.value()["axes"].get<decltype(default_sqaxes)>() ; 
            default_sqpower = v.value()["shapes"].get<decltype(default_sqpower)>() ;
        }
        else if (v.key() == "extra fields") { process_extrafields(v.value()) ; }
        else if (v.key() == "dimension") {dim = v.value().get<int>() ; requiredfieldset[v.key()] = true ; }
        else printf("Unknown json key: %s.\n", v.key().c_str()) ;
    }


/*
    post_init() ;
*/
}
//-----------------------------------------------
int Param::read_timestep (int ts, bool particleonly)
{
    if (curts == ts && tsread) return 0 ; //Already read and all
    printf("%d ", ts) ;fflush(stdout) ;
    for (auto & v: files)
    {
     v.reader->read_timestep(ts) ;
    }
    return 1 ;
}
//-------------------------------
void Param::post_init()
{
    if (files.size() ==0) { printf("ERR: a file should be defined.\n"); return ; }
    if (dim == -1 || !requiredfieldset["dimension"] ) dim=files[0].reader->get_dimension() ;
    if (!requiredfieldset["window size"]) { printf("ERR: 'window size' is required and cannot be built. \n") ; return ; }
    if (!requiredfieldset["fields"]) { printf("ERR: 'fields' is required and cannot be built. \n") ; return ; }
    if (!requiredfieldset["boxes"]) { printf("ERR: the number of boxes in each dimension is required and cannot be built.\n") ; return ; }
    if (!requiredfieldset["max time"])
    {
        maxT = files[0].reader->get_numts() ;
        printf("//%d %d//", maxT, skipT) ;
        if (maxT==-1)
        {
            printf("ERR: Cannot find the total number of timestep from the file.\n") ;
            return ;
        }
        maxT -= skipT ;
        printf("//%d//", maxT) ;
        if (maxT<0) maxT=0 ;
    }
    if (!requiredfieldset["boundaries"]) boundaries= files[0].reader->get_bounds() ;

    if ((boundaries.size() != 2 && boundaries.size() != 3) || boundaries[0].size() != static_cast<unsigned int>(dim) || boundaries[1].size()!=static_cast<unsigned int>(dim))
        printf("ERR: dimension of the boundaries is not consistent (should be '2 x dimension' or '3 x dimension' if including delta)\n") ;
    if (boxes.size() != static_cast<unsigned int>(dim))
        printf("ERR: dimension of the boxes is not consistent (should be 'dimension')\n") ;

    if (periodicity.size()>0)
    {
      bool hasper=false ;
      for (auto v : periodicity)
        if (v)
          hasper=true ;
      if (!hasper)
        periodicity.resize(0) ; // A periodic vector was defined, but with no PBC ... removing it.
      else
      {
        delta.resize(dim,0) ;

        for(int i=0 ; i<dim ; i++)
        {
          if (periodicity[i])
          {
            if (boxes[i]!=1)
              printf("WARN: using more than 1 box in the periodic dimension does not make much sense\n");
            delta[i] = boundaries[1][i]-boundaries[0][i] ;
          }
        }
      }
    }

    if (default_density!=-1)
    {
        for (auto &v: files)
            v.reader->set_default_density(default_density) ;
    }
    if (default_radius!=-1)
        for (auto &v: files)
            v.reader->set_default_radius(default_radius) ;
    
    if (default_sqaxes[0] != -1) 
        for (auto &v: files)
            v.reader->set_default_superquadric(default_sqaxes[0], default_sqaxes[1], default_sqaxes[2], default_sqpower[0], default_sqpower[1], default_sqpower[2]) ;

    for (auto &v: files)
      v.reader->post_init() ;
}
//---------------------------------------------------
int Param::identify_max_level()
{
    printf("THIS FUNCTION HAS BEEN REMOVED (identify_max_level)\n") ;
return -1 ;
}
//----------------------------------------------------------
Windows Param::identify_window(std::string windowstr)
{
         if ( windowstr=="Rect3D") return Windows::Rect3D ;
    else if ( windowstr=="Rect3DIntersect")  {printf("====> DEPRECATED: misleading name, use Sphere3DIntersect instead. <=======\n") ; return Windows::Sphere3DIntersect ;}
    else if ( windowstr=="Sphere3DIntersect") return Windows::Sphere3DIntersect ;
    else if ( windowstr=="SphereNDIntersect") return Windows::SphereNDIntersect ;
    else if ( windowstr=="Sphere3DIntersect_MonteCarlo") return Windows::Sphere3DIntersect_MonteCarlo ; 
    else if ( windowstr=="Lucy3D") return Windows::Lucy3D ;
    else if ( windowstr=="Hann3D") return Windows::Hann3D ;
    else if ( windowstr=="RectND") return Windows::RectND ;
    else if ( windowstr=="LucyND") return Windows::LucyND ;
    else if ( windowstr=="LucyND_Periodic") return Windows::LucyND_Periodic ;
    else if ( windowstr=="Lucy3DFancyInt")  return Windows::Lucy3DFancyInt;
    else if ( windowstr=="RVE")  return Windows::RVE;
    else {printf("Unknown windowing function.\n") ; return Windows::Lucy3D ; }
}
//-------------------------------------
void Param::process_extrafields (json &j)
{
    for (auto & v : j)
    {
        FieldType typ=FieldType::Particle ;
             if (v["type"].get<string>()=="Particle") typ = FieldType::Particle ;
        else if (v["type"].get<string>()=="Contact") typ = FieldType::Contact ;
        else if (v["type"].get<string>()=="Fluctuation") typ = FieldType::Fluctuation ;
        else printf("ERR: unknown extrafields type\n") ;
        extrafields.push_back({.name=v["name"].get<string>(), .order=static_cast<TensorOrder>(v["tensor order"].get<int>()), .type=typ}) ;
        if (v.exist("mapping")) {extrafields.back().mapping=v["mapping"].get<string>();}
    }
}
//---------------------------------------------------------
void Param::process_file (json &j2)
{
    for (size_t f=0 ; f<j2.size() ; f++)
    {
        auto & j =j2[f] ; 
        
        // Treat actions first if the tag is present
        if (j.exist("action"))
        {
            auto v = j["action"] ; 
            if ( v.get<string>() == "remove")
            {
                files.erase(files.begin()+f) ;
                continue ; 
            }
            else if (v.get<string>() == "edit")
                files.erase(files.begin()+f) ;
            else if (v.get<string>() == "donothing")
                continue ; 
            else if (v.get<string>() == "create") 
                ; // do nothing
            else
                printf("WARN: Unknown action command on process_file\n") ; 
        }         
        
        FileType content = FileType::particles ;
        if (j.exist("content"))
        { 
          if ( j["content"].get<string>() == "particles") content = FileType::particles ;
          else if ( j["content"].get<string>() == "contacts") content = FileType::contacts ;
          else if ( j["content"].get<string>() == "both") content = FileType::both ;
          else printf("WARN: unknown file content.\n") ;
        }
        else {printf("WARN: the key 'content' is recommended when defining a file. Set to 'particles' by default.\n") ;}

        FileFormat format ;
        if (j.exist("format"))
        { 
          if ( j["format"].get<string>() == "liggghts") format = FileFormat::Liggghts ;
          else if ( j["format"].get<string>() == "NDDEM") format = FileFormat::NDDEM ;
          else if ( j["format"].get<string>() == "mercury_legacy") format = FileFormat::MercuryData ;
          else if ( j["format"].get<string>() == "mercury_vtu") format = FileFormat::MercuryVTU ;
          else if ( j["format"].get<string>() == "yade") format = FileFormat::Yade ; 
          else if ( j["format"].get<string>() == "interactive") format = FileFormat::Interactive ;
          else {printf("ERR: unknown file format.\n") ; return ;}
        }
        else { printf("ERR: the key 'format' is required when defining a file. \n") ; return ; }

        
        std::map <std::string, std::string> mapping ;
        if ( j.exist("mapping") ) mapping= j["mapping"].get<std::map <std::string, std::string>>();

        files.insert(files.begin()+f, {.fileformat=format, .filetype=content, .mapping=mapping, .reader = nullptr,}) ;
     
        if (files[f].fileformat == FileFormat::NDDEM)
            files[f].reader = new NDDEMReader (j["filename"].get<string>());
        else if (files[f].fileformat == FileFormat::Interactive)
            files[f].reader = new InteractiveReader ();
        #ifndef NOTALLFORMATS
        else if (files[f].fileformat == FileFormat::Liggghts)
        {
            if (files[f].filetype == FileType::both)
                files[f].reader = new LiggghtsReader (j["filename"].get<string>()) ;
            else if (files[f].filetype == FileType::particles)
                files[f].reader = new LiggghtsReader_particles (j["filename"].get<string>()) ;
            else if (files[f].filetype == FileType::contacts)
            {
                std::vector<File>::iterator it ;
                for (it = files.begin() ; it<files.end() ; it++)
                    if (it->filetype==FileType::particles)
                        break ;
                if (it == files.end())
                {
                    printf("ERR: a dump particles needs to be defined before defining a dump contacts\n") ;
                    files.pop_back() ;
                    continue ;
                }
                files[f].reader = new LiggghtsReader_contacts (j["filename"].get<string>(), it->reader, files[f].mapping) ;
            }
        }
        else if (files[f].fileformat == FileFormat::MercuryData)
        {
            if (files[f].filetype == FileType::particles)
                files[f].reader = new MercuryReader_data_particles (j["filename"].get<string>()) ;
            else if (files[f].filetype == FileType::contacts) // TODO
            {}//files[f].reader = new MercuryReader_contacts (files[f].path) ;
            else
                printf("ERR: no Mercury file format support FileType::both.\n") ;
        }
        else if (files[f].fileformat == FileFormat::MercuryVTU)
        {
            if (files[f].filetype == FileType::particles)
                files[f].reader = new MercuryReader_vtu_particles (j["filename"].get<string>()) ;
            //else if (files[f].filetype == FileType::contacts) // TODO
            //{}//files[f].reader = new MercuryReader_contacts (files[f].path) ;
            else
                printf("ERR: no Mercury VTU file format support FileType::contacts and FileType::both.\n") ;
        }
        else if (files[f].fileformat == FileFormat::Yade)
        {
            files[f].reader = new YadeReader() ; 
        }
        #endif
        else
        {
            printf("Coarse graining has not been compiled with the requested file format") ;
        }
        
        bool multifile = false ; 
        files[f].reader->path = j["filename"].get<string>() ; 
        if (j.exist("initial")) {multifile = true ; files[f].reader->filenumbering.initial = j["initial"].get<double>() ; }
        if (j.exist("delta")) {multifile = true ; files[f].reader->filenumbering.delta = j["delta"].get<double>() ; }

        if (multifile && files[f].reader->path.find('%') == std::string::npos)
            printf("WARN: you have provided file initial or delta numbering, but there is no pattern in your filename, which is surprising\n") ; 
        if (files[f].reader->path.find('%') != std::string::npos) files[f].reader->filenumbering.ismultifile = true ;         
        
    }
}
//------------------
int Param::set_data(struct Data & D)
{
    D.N=get_num_particles() ; D.Ncf=get_num_contacts() ;
    D.radius = get_data(DataValue::radius) ;
    D.mass   = get_data(DataValue::mass) ;
    D.Imom   = get_data(DataValue::Imom) ;

    D.pos.resize(dim) ; D.vel.resize(dim) ;
    D.pospq.resize(dim) ; D.lpq.resize (dim) ;
    D.fpq.resize (dim) ;
    D.omega.resize(dim*(dim-1)/2);
    D.mpq.resize (dim*(dim-1)/2) ; D.mqp.resize (dim*(dim-1)/2) ;

    D.id1 = get_data(DataValue::id1);
    D.id2 = get_data(DataValue::id2);
    for (int dd=0 ; dd<dim ; dd++)
    {
        D.pos[dd]   = get_data(DataValue::pos, dd) ;
        D.vel[dd]   = get_data(DataValue::vel, dd) ;
        D.pospq[dd] = get_data(DataValue::pospq, dd) ;
        D.lpq[dd]   = get_data(DataValue::lpq, dd) ;
        D.fpq[dd]   = get_data(DataValue::fpq, dd) ;
    }
    for (int dd=0 ; dd<dim*(dim-1)/2 ; dd++)
    {
        D.omega[dd] = get_data(DataValue::omega, dd) ;
        D.mpq[dd]   = get_data(DataValue::mpq, dd) ;
        D.mqp[dd]   = get_data(DataValue::mqp, dd) ;
    }

    D.orient.resize(4) ;
    for (int dd=0 ; dd<4 ; dd++)
        D.orient[dd] = get_data(DataValue::orient, dd) ;
    
    D.superquadric.resize(6) ; 
    for (int dd=0 ; dd<6 ; dd++)
        D.superquadric[dd] = get_data(DataValue::superquadric, dd) ; 

    for (auto &v: D.extrafields)
    {
        for (int dd=0 ; dd<std::get<1>(v) ; dd++)
            D.extra[std::get<2>(v)+dd] = get_data(DataValue::extra_named, dd, std::get<0>(v)) ;
    }
    return 0 ;
}
//------------------------------------------------------------------------
int Param::get_num_particles ()
{
    for (auto &v: files)
        if (v.reader->get_num_particles() != -1)
            return v.reader->get_num_particles() ;
    return -1 ;
}
int Param::get_num_contacts ()
{
    for (auto &v: files)
        if (v.reader->get_num_contacts() != -1)
            return v.reader->get_num_contacts() ;
    return -1 ;
}
double Param::get_volume()
{
    auto bnds = files[0].reader->get_bounds() ;
    double volume = 1 ;
    for (size_t i=0; i<bnds[0].size() ; i++)
        volume *= bnds[1][i]-bnds[0][i] ;
    return volume ;
}
double * Param::get_data(DataValue datavalue, int dd, std::string name)
{
    double * res ;
    for (auto &v: files)
    {
        res = v.reader->get_data(datavalue, dd, name) ;
        if (res != nullptr)
            return res;
    }
    return nullptr ;
}
#endif
