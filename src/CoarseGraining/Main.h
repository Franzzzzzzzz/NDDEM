#include <cstdlib>
#include <cstdio>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <sstream>

#ifdef EMSCRIPTEN
    #include <emscripten.h>
    #include <emscripten/bind.h>
    using namespace emscripten ;
#endif

#include "gzip.hpp"
//#include "termcolor.hpp"
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/stream.hpp>
#include "json.hpp"
#include "Typedefs.h"
#include "Coarsing.h"
#include "Reader.h"
#include "Reader-Liggghts.h"
#include "Reader-NDDEM.h"
#include "Parameters.h"

class CoarseGraining {
public:
    CoarseGraining() {}
    Param   P ; 
    Coarsing * C = nullptr; 
    Pass pipeline ; 
    
    void setup_CG () ;
    
    int process_timestep (int ts, bool allow_avg_fluct=false) ;
    int process_fluct_from_avg() ;
    void process_all ();
    void write () ; 
    std::vector<double> get_result(int ts, std::string field, int component) ;
    std::vector<double> get_gridinfo() ;
    
    // Expose functions from member classes for js access
    void param_from_json_string (std::string param)  { json jsonparam =json::parse(param) ; return P.from_json(jsonparam) ; }
    std::vector<std::vector<double>> param_get_bounds (int file = 0) {return P.files[file].reader->get_bounds() ; }
    int param_get_numts(int file = 0) {return P.files[file].reader->get_numts(); }
    void debug () {/*char resc[5000] ; sprintf(resc, "%d %X", P.files.size(), P.files[0].reader) ; return resc ; */printf("BLAAAA\n") ; }
    int param_read_timestep(int n) {return P.read_timestep(n) ; }
    void param_post_init () {return P.post_init() ; }
    
    /*std::string testing(std::string input) { 
        FILE * in = fopen(input.c_str(), "r") ; 
        printf("[%s]", input.c_str()) ;
        if (in != nullptr)
        {
            printf("Starting reading\n") ; 
            while (!feof(in))
                fscanf(in,"%*c") ; 
            fclose(in) ;
            printf("Finished reading\n") ; 
        }
        return(input) ; 
    } */
    
} ;
//=========================================================
