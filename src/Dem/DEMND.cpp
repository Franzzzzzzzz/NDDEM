/** \addtogroup DEM Discrete Element Simulations
 * This module handles the Discrete Element Simulations.
 *  @{ */

#include "DEMND.h"
#include <signal.h>
//#include <gperftools/profiler.h>
// #include "Benchmark.h"
//#define OMP_NUM_THREADS 2

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
    S.finalise_init() ;

    S.step_forward() ;

    S.finalise() ;

return 0 ;
}


//===================================================
/** \brief Calls the appropriate templatedmain() function. Templated function are used to allow compiler optimisation for speed. Only a handful of dimension are compiled on the base code to limit compilation time and memory. If you need dimensions that are not cmpiled by default, have a look at the code it's pretty straightforward to activate the needed dimension. If the compilation failed with low meomry, in particular on older system, head to the code and comment the dimensions which are unused.
 */
int main (int argc, char *argv[])
{
  /* TESTING */
  v2d a={{0.56643248, 0.26125725, 0.73497898, 0.28643289, 0.92855862,
        0.8699034 , 0.30697165, 0.64215012},
       {0.62203212, 0.8012233 , 0.59427183, 0.01558617, 0.72446953,
        0.08977969, 0.91596307, 0.34652542},
       {0.05191553, 0.26019937, 0.59655408, 0.59934892, 0.99911472,
        0.75718539, 0.74470339, 0.21995364},
       {0.36407997, 0.23997137, 0.72158439, 0.79213163, 0.05416148,
        0.71473292, 0.66847004, 0.40290451},
       {0.78675797, 0.14196326, 0.34941157, 0.8129907 , 0.78384772,
        0.93352639, 0.93152656, 0.87937325},
       {0.3885721 , 0.33088209, 0.45171187, 0.4272163 , 0.12074053,
        0.41378471, 0.1903225 , 0.20809885},
       {0.75327964, 0.95418903, 0.51130511, 0.19382191, 0.15099623,
        0.19725078, 0.27185385, 0.82699336},
       {0.15850575, 0.66685922, 0.9059653 , 0.64678518, 0.98700726,
        0.87285161, 0.38082877, 0.20302647}};
  constexpr int d=8 ;
  Tools<d>::initialise() ;
  Tools<d>::inv_Gauss(a) ;
  std::exit(0) ;


 signal (SIGINT, sig_handler);   // Catch all signals ...

 if (argc<4) {printf("Usage: DEMND #dimensions #grains inputfile\n") ; std::exit(1) ; }
 int dd=atoi(argv[1]) ;

 switch (dd)
 {
//     case  1: templatedmain<1> (argv) ; break ;
     case  2: templatedmain<2> (argv) ; break ;
     case  3: templatedmain<3> (argv) ; break ;
     case  4: templatedmain<4> (argv) ; break ;
//     case  5: templatedmain<5> (argv) ; break ;
//     case  6: templatedmain<6> (argv) ; break ;
//     case  7: templatedmain<7> (argv) ; break ;
//     case  8: templatedmain<8> (argv) ; break ;
//      case  9: templatedmain<9> (argv) ; break ;
//      case 10: templatedmain<10> (argv) ; break ;
//      case 11: templatedmain<11> (argv) ; break ;
//      case 12: templatedmain<12> (argv) ; break ;
//      case 13: templatedmain<13> (argv) ; break ;
//      case 14: templatedmain<14> (argv) ; break ;
//      case 15: templatedmain<15> (argv) ; break ;
//      case 16: templatedmain<16> (argv) ; break ;
//      case 17: templatedmain<17> (argv) ; break ;
//      case 18: templatedmain<18> (argv) ; break ;
//      case 19: templatedmain<19> (argv) ; break ;
//      case 20: templatedmain<20> (argv) ; break ;
//      case 21: templatedmain<21> (argv) ; break ;
//      case 22: templatedmain<22> (argv) ; break ;
//      case 23: templatedmain<23> (argv) ; break ;
//      case 24: templatedmain<24> (argv) ; break ;
//      case 25: templatedmain<25> (argv) ; break ;
     default : printf("DEMND was not compiled with support for dimension %d. Please recompile modifying the main function to support that dimension.\n", dd); std::exit(1) ;
 }
printf("b") ;
return 0 ;
}

/** @} */
