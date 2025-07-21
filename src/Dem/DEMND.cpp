/** \addtogroup DEM Discrete Element Simulations
 * This module handles the Discrete Element Simulations.
 *  @{ */

#include "DEMND.h"
#include <signal.h>
//#include <gperftools/profiler.h>
// #include "Benchmark.h"
//#define OMP_NUM_THREADS 2

// Preprocessing command to compile only enough dimensions as needed. 
#ifndef MAXDIM 
#define MAXDIM 4
#endif
#include "Preprocessor_macros.h"
#define MACRO(r, state) \
    case BOOST_PP_TUPLE_ELEM(2, 0, state): templatedmain<BOOST_PP_TUPLE_ELEM(2, 0, state)>(argv);break;

//===================================================================================================
vector <std::pair<ExportType,ExportData>> * toclean ;
XMLWriter * xmlout ;

/** \brief Function handling signal reception for clean closing on SIGINT for example.
 */
void sig_handler (int p)
{
    // Benchmark::write_all() ;
    for (auto v : *toclean)
        if ((v.first == ExportType::XML) || (v.first == ExportType::XMLbase64))
            xmlout->emergencyclose() ;
    //printf("\n\n\n\n\n\n\n") ;
    std::exit(p) ;
}

template <int d>
void save_restart(const Simulation<d> &S, const std::string &filename) {
    std::ofstream os(filename, std::ios::binary);
    cereal::BinaryOutputArchive archive(os);
    archive(S);
}

/** \brief Main simulation run. All the simulation is handled by this function.
 */
template <int d>
int templatedmain (char * argv[])
{
    int NN=atoi(argv[2]) ;
    Simulation<d> S(NN) ;
    if (strcmp(argv[3], "default"))
        S.init_from_file (argv[3]) ;
    S.finalise_init() ;

    S.step_forward_all() ;

    S.finalise() ;

return 0 ;
}


//===================================================
/** \brief Calls the appropriate templatedmain() function. Templated function are used to allow compiler optimisation for speed. Only a handful of dimension are compiled on the base code to limit compilation time and memory. If you need dimensions that are not cmpiled by default, have a look at the code it's pretty straightforward to activate the needed dimension. If the compilation failed with low meomry, in particular on older system, head to the code and comment the dimensions which are unused.
 */
int main (int argc, char *argv[])
{
 signal (SIGINT, sig_handler);   // Catch all signals ...

 // TEST
/* v1d A = {0.8147,    0.6324,    0.9575,    0.9572,    0.9058,    0.0975,    0.9649,    0.4854,
    0.1270,    0.2785,    0.1576,    0.8003,    0.9134,    0.5469,    0.9706,    0.1419} ; 
 Tools<4>::print(A) ; 
    Tools<4>::print(Tools<4>::inverse(A)) ; 
 
 std::exit(0) ;*/ 
 // END TEST
 
 
 
 if (argc<4) {printf("Usage: DEMND #dimensions #grains inputfile\n") ; std::exit(1) ; }
 int dd=atoi(argv[1]) ;

 switch (dd)
 {
     BOOST_PP_FOR((1, MAXDIM), PRED, OP, MACRO)
     
     default : printf("DEMND was not compiled with support for dimension %d. Please recompile modifying the main function to support that dimension.\n", dd); std::exit(1) ;
 }

return 0 ;
}

/** @} */
