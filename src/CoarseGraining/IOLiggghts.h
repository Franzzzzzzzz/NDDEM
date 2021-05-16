#include <cstdlib>
#include <cstdio>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <sstream>

#include "gzip.hpp"
//#include "termcolor.hpp"
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/stream.hpp>
#include "json.hpp"
#include "Typedefs.h"
#include "Coarsing.h"

using namespace std;
using json = nlohmann::json;


class Datafile {
public:
    Datafile() : Radius(0.00075), Rho(2500) {}
    ~Datafile() {
        if (file_in   != nullptr && file_in->is_open()) file_in->close() ;
        if (file_incf != nullptr && file_incf->is_open()) file_incf->close() ;
    }


    istream *in = nullptr, *incf = nullptr ; //, *incf ;

    int curts ; int N, Ncf ;
    double Radius, Rho ;

    vector <string> fields, fieldscf ;
    v2d data, tdata, datacf ;
    v1d dataextra ;

    // Functions
    int open(string path) ;
    int reopen(string path) ;
    int opencf(string path) ;
    int reopencf(string path) ;
    int read_full_ts(bool keep) ;
    int set_data(struct Data & D, std::map<string,size_t> extrafieldmap) ;
    vector<vector<double>> get_bounds () ;
    vector <vector <double> > boundaries ;
    vector<bool> periodicity ;
    vector<double> delta ;
    int get_numts () ;

    std::map <std::string, std::string> cfmapping ;

private:
    int read_next_ts(istream *is, bool iscf, bool keep) ;
    int do_post_atm() ;
    int do_post_cf() ;
    boost::iostreams::filtering_streambuf<boost::iostreams::input> filt_in;
    boost::iostreams::filtering_streambuf<boost::iostreams::input> filt_in2;
    boost::iostreams::filtering_streambuf<boost::iostreams::input> filt_incf;
    boost::iostreams::filtering_streambuf<boost::iostreams::input> filt_incf2;
    ifstream * file_in = nullptr , *file_incf = nullptr  ;
} ;

// ========================
struct Param {
public:
  string atmdump="" ;
  string cfdump="" ;
  int skipT=0 ;
  int maxT =1;
  vector <string> flags = {} ;
  const int dim=3 ;
  vector <int> boxes= {3,3,3} ;
  vector <vector <double> > boundaries={{0,0,0},{1,1,1}} ;
  string save="" ;
  vector<string> saveformat ;
  std::map <std::string, std::string> cfmapping ;

  bool hascf = false ;
  bool dotimeavg = false ;
  uint8_t maxlevel = 3 ;

  void from_json (json & j) ;
  string windowstr = "";
  Windows window = Windows::Lucy3D ;
  double windowsize ;
  vector<bool> periodicity ;
  vector<double>delta ;

  struct ExtaField {
    string name  ;
    TensorOrder order ;
    FieldType type ;
  } ;
  vector<ExtaField> extrafields ;

private :
  bool needmaxts = true ;
  bool needbounds = true ;
  void post_init () ;

} ;

//-------------------------------
void Param::from_json(json &j)
{
    auto v = j.find("particles") ;
    if (v == j.end()) { printf("The key 'particles' is required\n") ; std::exit(3) ; }
    else atmdump = v->get<string>();

    v = j.find("savefile") ;
    if (v == j.end()) { printf("The key 'savefile' is required\n") ; std::exit(3) ; }
    else save = v->get<string>();

    v = j.find("saveformat") ;
    if (v == j.end()) { printf("The key 'saveformat' is required\n") ; std::exit(3) ; }
    else
    {
      try {saveformat = v->get<vector<string>>();}
      catch(...) {string a = v->get<string>(); saveformat.push_back(a) ; }
    }

    v = j.find("window size") ;
    if (v != j.end()) windowsize = v->get<double>();
    else { printf("The key 'window size' is required\n") ; std::exit(3) ; }

    v = j.find("forces") ;
    if (v != j.end()) {cfdump = v->get<string>(); hascf = true ; }

    v = j.find("skip") ;
    if (v != j.end()) skipT = v->get<int>();

    v = j.find("max time") ;
    if (v != j.end()) { maxT = v->get<int>(); needmaxts = false ; }
    else printf("The 'max time' key is strongly recommended, the file will have to be processed twice otherwise.\n") ;

    v = j.find("fields") ;
    if (v != j.end()) flags = v->get<vector<string>>();
    else printf("The 'fields' key is strongly recommended, nothing will be processed otherwise.\n") ;

    v = j.find("boxes") ;
    if (v != j.end()) boxes = v->get<decltype(boxes)>();

    v = j.find("mapping") ;
    if (v != j.end()) cfmapping= v->get<decltype(cfmapping)>();

    v = j.find("boundaries") ;
    if (v != j.end()) {boundaries = v->get<decltype(boundaries)>(); needbounds=false ; }
    else {printf("'Boundaries' are not defined, the whole simulation box will be used.\n") ;}

    v = j.find("time average") ;
    if (v != j.end()) {dotimeavg = v->get<bool>(); }

    v = j.find("window") ;
    if (v != j.end()) windowstr = v->get<string>();

    v = j.find("periodicity") ;
    if (v != j.end()) periodicity = v->get<decltype(periodicity)>();

    v = j.find("extra fields") ;
    if (v != j.end())
    {
      auto v2=j["extra fields"] ;
      extrafields.resize(v2.size()) ;
      for (size_t i=0 ; i<v2.size() ; i++)
      {
        extrafields[i].name = v2[i]["name"] ;
        extrafields[i].order = static_cast<TensorOrder>(v2[i]["tensor order"].get<int>()) ;

        if (v2[i]["type"]=="Particle")
          extrafields[i].type=FieldType::Particle ;
        else if (v2[i]["type"]=="Contact")
          extrafields[i].type=FieldType::Contact ;
        else if (v2[i]["type"]=="Fluctuation")
          extrafields[i].type=FieldType::Fluctuation ;
        else
          printf("ERR: unknown extrafields type\n") ;
      }
    }


    post_init() ;
}

//-------------------------------
void Param::post_init()
{
    if (boundaries.size() != 2 || boundaries[0].size() != static_cast<size_t>(dim) || boundaries[1].size() != static_cast<size_t>(dim) )
    { printf("Inconsistent boundaries\n") ; std::exit(3) ; }

    if (!hascf && (
        std::find(flags.begin(), flags.end(), "TC") != flags.end() ||
        std::find(flags.begin(), flags.end(), "MC") != flags.end() ||
        std::find(flags.begin(), flags.end(), "mC") != flags.end() ||
        std::find(flags.begin(), flags.end(), "qTC") != flags.end() ||
        std::find(flags.begin(), flags.end(), "qRC") != flags.end()
    ))
    { printf("Contact force informations are required for one or more of the fields you requested\n") ; std::exit(3) ; }

    if (std::find(flags.begin(), flags.end(), "RHO"  ) != flags.end() ||
        std::find(flags.begin(), flags.end(), "VAVG" ) != flags.end() ||
        std::find(flags.begin(), flags.end(), "ROT"  ) != flags.end() ||
        std::find(flags.begin(), flags.end(), "EKT"  ) != flags.end() ||
        std::find(flags.begin(), flags.end(), "EKR"  ) != flags.end())
    maxlevel=1 ;

    if (std::find(flags.begin(), flags.end(), "TK"   ) != flags.end() ||
        std::find(flags.begin(), flags.end(), "MK"   ) != flags.end() ||
        std::find(flags.begin(), flags.end(), "eKT"  ) != flags.end() ||
        std::find(flags.begin(), flags.end(), "eKR"  ) != flags.end() ||
        std::find(flags.begin(), flags.end(), "qTK"  ) != flags.end() ||
        std::find(flags.begin(), flags.end(), "qRK"  ) != flags.end())
    maxlevel = 2 ;

    if (std::find(flags.begin(), flags.end(), "TC"   ) != flags.end() ||
        std::find(flags.begin(), flags.end(), "MC"   ) != flags.end() ||
        std::find(flags.begin(), flags.end(), "mC"   ) != flags.end() ||
        std::find(flags.begin(), flags.end(), "qTC"  ) != flags.end() ||
        std::find(flags.begin(), flags.end(), "qRC"  ) != flags.end() ||
        std::find(flags.begin(), flags.end(), "zT"   ) != flags.end() ||
        std::find(flags.begin(), flags.end(), "zR"   ) != flags.end())
    maxlevel = 3 ;

    if (std::find(flags.begin(), flags.end(), "TotalStress"   ) != flags.end() ||
        std::find(flags.begin(), flags.end(), "Pressure"   ) != flags.end() ||
        std::find(flags.begin(), flags.end(), "KineticPressure"   ) != flags.end() ||
        std::find(flags.begin(), flags.end(), "ShearStress"  ) != flags.end() ||
        std::find(flags.begin(), flags.end(), "StrainRate"  ) != flags.end() ||
        std::find(flags.begin(), flags.end(), "VolumetricStrainRate"   ) != flags.end() ||
        std::find(flags.begin(), flags.end(), "ShearStrainRate"   ) != flags.end())
    maxlevel = 4 ;
    
    if (needbounds)
    {
        Datafile D ; // Temporary datafile in principle the destructor handles the file closing ...
        D.open(atmdump) ;
        boundaries = D.get_bounds() ;
    }

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
        if (needbounds==false)
          printf("WARN: you should really not provide the bounds if you are expecting to use periodicity.\n") ;

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

    if (needmaxts)
    {
        Datafile D ; // Temporary datafile in principle the destructor handles the file closing ...
        D.open(atmdump) ;
        maxT = D.get_numts() ;
    }

    if (windowstr != "")
    {
     if (windowstr=="Rect3D") window=Windows::Rect3D ;
     else if ( windowstr=="Rect3DIntersect") window=Windows::Rect3DIntersect ;
     else if ( windowstr=="Lucy3D") window=Windows::Lucy3D ;
     else if ( windowstr=="Hann3D") window=Windows::Hann3D ;
     else if ( windowstr=="RectND") window=Windows::RectND ;
     else if ( windowstr=="LucyND") window=Windows::LucyND ;
     else if ( windowstr=="LucyND_Periodic") window=Windows::LucyND_Periodic ;
     else if ( windowstr=="Lucy3DFancyInt") window=Windows::Lucy3DFancyInt; 
     else {printf("Unknown windowing function.\n") ; std::exit(4) ; }
    }
}
