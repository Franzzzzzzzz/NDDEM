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

    // Functions
    int open(string path) ;
    int opencf(string path) ;
    int read_full_ts(bool keep) ;
    int set_data(struct Data & D) ;
    vector<vector<double>> get_bounds () ; 
    int get_numts () ; 
    
    std::map <std::string, std::string> cfmapping ; 

private:
    int read_next_ts(istream *is, bool iscf, bool keep) ;
    int do_post_atm() ;
    int do_post_cf() ;
    boost::iostreams::filtering_streambuf<boost::iostreams::input> filt_in;
    boost::iostreams::filtering_streambuf<boost::iostreams::input> filt_incf;
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
  string saveformat="" ; 
  std::map <std::string, std::string> cfmapping ; 
  
  bool hascf = false ; 
  bool dotimeavg = false ; 
  uint8_t maxlevel = 3 ; 
  
  void from_json (json & j) ;
  string windowstr = ""; 
  Windows window = Windows::Lucy3D ; 
  double windowsize ; 

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
    else saveformat = v->get<string>();
    
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
    
    post_init() ; 
}

//-------------------------------
void Param::post_init()
{
    if (boundaries.size() != 2 || boundaries[0].size() != dim || boundaries[1].size() != dim )
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
    
    if (needbounds)
    {
        Datafile D ; // Temporary datafile in principle the destructor handles the file closing ...
        D.open(atmdump) ; 
        boundaries = D.get_bounds() ; 
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
     else {printf("Unknown windowing function.\n") ; std::exit(4) ; }
    }
}








