/** \addtogroup DEM Discrete Element Simulations
 * This module handles the Discrete Element Simulations.
 *  @{ */

#include "DEMND.h"
#include <signal.h>
#include <regex>
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

/** \brief Main simulation run. All the simulation is handled by this function.
 */
template <int d>
int templatedmain (char * argv[])
{
    int NN=atoi(argv[2]) ;
    Simulation<d> S(NN) ;
    if (strcmp(argv[3], "default"))
        S.init_from_file (argv[3]) ;
    
    bool restarting = false ; 
    if (S.P.n_restart>=0 && !((S.P.restart_flag & (1<<1))>0)) // Need to consider whether we should restart from file
    {
        if (S.P.restart_flag & (2<<1)) // force restart
        {
            S.load_restart(S.P.Directory + "/" + S.P.restart_filename) ;
            S.ti++ ;
            restarting = true ; 
        }
        else //auto
        {
          std::optional<std::string> latest_file;
          std::filesystem::file_time_type latest_time;
          
          if (S.P.restart_flag&1) 
          {
            std::regex pattern(S.P.restart_filename + "-\\d+");
            std::string dir_path = S.P.Directory ;
            for (const auto& entry : std::filesystem::directory_iterator(dir_path)) 
            {
              if (!entry.is_regular_file()) continue;

              const std::string name = entry.path().filename().string();
        
              if (std::regex_match(name, pattern)) 
              {
                printf("FOUND %s\n", name.c_str()) ; 
                auto ftime = fs::last_write_time(entry);
                if (!latest_file || ftime > latest_time) 
                {
                    latest_time = ftime;
                    latest_file = name;
                }
              }
            }
          }
          else 
          {
            std::string even_file = S.P.Directory + "/" + S.P.restart_filename + "-even";
            std::string odd_file  = S.P.Directory + "/" + S.P.restart_filename + "-odd";
            bool even_exists = fs::exists(even_file);
            bool odd_exists  = fs::exists(odd_file);

            if (even_exists && (!odd_exists || fs::last_write_time(even_file) > fs::last_write_time(odd_file))) 
                latest_file = even_file;
            else if (odd_exists)
                latest_file = odd_file;
         }
         
         if (latest_file.has_value())
         {
           S.load_restart(S.P.Directory + "/" + latest_file.value()) ; 
           S.ti++ ;
           S.t += S.dt ;
           restarting = true ; 
         }
       }
    }
    
    if (restarting == false)
    { 
      S.finalise_init() ;
      S.step_forward_all() ;
      S.finalise() ;
    }
    else
    {
      S.step_forward((S.P.T-S.t)/S.P.dt) ; 
      S.finalise() ;
    }

return 0 ;
}


//===================================================
/** \brief Calls the appropriate templatedmain() function. Templated function are used to allow compiler optimisation for speed. Only a handful of dimension are compiled on the base code to limit compilation time and memory. If you need dimensions that are not cmpiled by default, have a look at the code it's pretty straightforward to activate the needed dimension. If the compilation failed with low meomry, in particular on older system, head to the code and comment the dimensions which are unused.
 */
int main (int argc, char *argv[])
{
 signal (SIGINT, sig_handler);   // Catch all signals ...

 // TEST
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
