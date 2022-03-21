#include <cstdlib>
#include <cstdio>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <sstream>

//#include "gzip.hpp"
//#include "termcolor.hpp"
//#include <boost/iostreams/filtering_streambuf.hpp>
//#include <boost/iostreams/stream.hpp>

#include "Typedefs.h"
#include "Coarsing.h"
#include "../Dem/Xml.h"

using namespace std;


//==========================================================
/** \brief Contains the parameters of the simulation & CG
 */
struct Param {
  string dump ;
  int skipT ;
  int maxT ;
  double rho ;
  vector <string> flags ;
  vector <int> boxes ;
  vector <vector <double> > boundaries ;
  int pbc = 0 ;
  vector<double> Delta ;
  vector <double> radius ;
  string save = "CoarseGrained";
  double windowsize = 1 ;
  double cuttoff = 2.5 ;

  void from_file(char path[])
  {
    ifstream in ;

    in.open(path) ;
    if (!in.is_open()) { printf("[Input] file cannot be open\n"); return ;}

    while (! in.eof())
      parsing(in) ;
  }
  void disp()
  {
    auto pbcprint = [](int a) {string s ="" ; for (;a>0; a>>=1) s = ((a&1)?"y":"n") + s ; return s ;} ;
    printf("\n-----\n%s\nSkipping: %d\nFinal time: %d\nDensity: %g\nFlags: ", dump.c_str(), skipT, maxT, rho) ;
    for (auto v: flags) printf("%s ", v.c_str()) ;
    printf("\nBoxes: ") ;
    for (auto v: boxes) printf("%d ", v) ;
    printf("\nPBCs: %s\nWindowSize: %g\nCut-off: %g\nDeltas: ", pbcprint(pbc).c_str(), windowsize, cuttoff) ;
    for (auto v: Delta) printf("%g ", v) ;
    printf("\nBoundaries:") ;
    for (auto v:boundaries)
    {
      printf("\n") ;
      for (auto w:v)
        printf("%g ", w);
    }
    printf("\n-----\n\n") ; fflush(stdout) ;
  }
private:
  int parsing (istream & in) ;
} ;


//--------------------------------------------
int Param::parsing (istream & in)
{
  char line[5000] ; int id ; int rien, dimension ; double mass, radius ;

  in>>line;
  if (line[0]=='#') {in.getline(line, 5000) ; return 0; } // The line is a comments

  if (!strcmp(line, "directory"))
  {
    in>>dump ;
    save = dump + "/" + save ;
    dump += "/dump.xml" ;
    if (! experimental::filesystem::exists(dump))
    {
      printf("[ERR] file do not exist: %s\n", dump.c_str());
    }
  }
  else if (!strcmp(line, "dimensions"))
  {
    in >> dimension ; in >> rien ;
    Delta.resize(dimension, 0) ;
    boxes.resize(dimension, 0) ;
    boundaries.resize(2, vector<double> (dimension, 0)) ;
  }
  else if (!strcmp(line, "boundary"))
  {
    int walldim ; char type[50] ; double dmin, dmax ;
    in >> walldim ; in >> type ; in>>dmin ; in >> dmax ;
    boundaries[0][walldim]= dmin ;
    boundaries[1][walldim]= dmax ;
    if (!strcmp(type, "PBC"))
    {
      pbc |= (1<<walldim) ;
      Delta[walldim] = dmax-dmin ;
    }
    else {} // Other types do not matter
  }
  else if (!strcmp(line, "gravity")) {in.getline(line, 5000) ; return 1; }
  else if (!strcmp(line, "set"))
  {
    in >> line ;
    if (!strcmp(line, "rho")) in >> rho ;
    else if (!strcmp(line, "Kn")) {in.getline(line, 5000) ; return 1;}
    else if (!strcmp(line, "Kt")) {in.getline(line, 5000) ; return 1;}
    else if (!strcmp(line, "GammaN")) {in.getline(line, 5000) ; return 1;}
    else if (!strcmp(line, "GammaT")) {in.getline(line, 5000) ; return 1;}
    else if (!strcmp(line, "Mu")) {in.getline(line, 5000) ; return 1;}
    else if (!strcmp(line, "T")) {in.getline(line, 5000) ; return 1;}
    else if (!strcmp(line, "tdump")) {in.getline(line, 5000) ; return 1;}
    else if (!strcmp(line, "orientationtracking")) {in.getline(line, 5000) ; return 1;}
    else if (!strcmp(line, "skin")) {in.getline(line, 5000) ; return 1;}
    else if (!strcmp(line, "dumps")) {in.getline(line, 5000) ; return 1;}
    else if (!strcmp(line, "tinfo")) {in.getline(line, 5000) ; return 1;}
    else if (!strcmp(line, "dt")) {in.getline(line, 5000) ; return 1;}
  }
  else if (!strcmp(line, "radius"))
  {
    int id ; double value ;
    in >> id ; in>>value ;
    if (id==-1 || id ==0) radius=value ;
  }
  else if (!strcmp(line, "mass"))
  {
    int id ; double value ;
    in >> id ; in>>value ;
    if (id==-1 || id ==0) mass=value ;
  }
  else if (!strcmp(line, "auto"))
  {
    in >> line ;
    if (!strcmp(line, "rho"))
    {
      rho  = mass / Volume(dimension, radius) ;
    }
    else if (!strcmp(line, "location")) {in.getline(line, 5000) ; return 1; } // Do not matter
    else if (!strcmp(line, "inertia")) {in.getline(line, 5000) ; return 1 ; } // Do not matter
    else if (!strcmp(line, "mass")) {in.getline(line, 5000) ; return 1 ; } // Do not matter
  }
  else if (!strcmp(line, "CG"))
  {
    in>>line ;
    if (!strcmp(line, "skiptime")) in >> skipT ;
    else if (!strcmp(line, "maxtime")) in >> maxT ;
    else if (!strcmp(line, "flags"))
    {
      int nb ; in >> nb ;
      for (int i=0 ; i<nb ; i++)
      {
        in >> line ;
        flags.push_back(line) ;
      }
    }
    else if (!strcmp(line, "boxes"))
      for (auto &v : boxes)
        in>>v ;
    else if (!strcmp(line, "bound"))
    {
      int dim ; double bmin, bmax ;
      in>>dim >> bmin >> bmax ;
      boundaries[0][dim] = bmin ;
      boundaries[1][dim] = bmax ;
    }
    else if (!strcmp(line, "radius"))
    {} // TODO
    else if (!strcmp(line, "windowsize"))
    {
      in >> windowsize ;
      cuttoff = 2*windowsize ;
    }
    else if (!strcmp(line, "cutoff"))
    {
      in >> cuttoff ;
    }
    else
      printf("[Input] Unknown command in input file |CG %s|\n", line) ;
  }
  else if (!strcmp(line, "location")){in.getline(line, 5000) ;  return 1 ; }
  else if (!strcmp(line, "velocity")){in.getline(line, 5000) ; return 1 ; }
  else if (!strcmp(line, "omega")){in.getline(line, 5000) ; return 1 ; }
  else if (!strcmp(line, "freeze")){in.getline(line, 5000) ; return 1 ; }
  else if (!strcmp(line, "gravity")){in.getline(line, 5000) ; return 1 ; }
  else if (!strcmp(line, "gravityangle")){in.getline(line, 5000) ; return 1 ; }
  else if (!strcmp(line,"event")) {in.getline(line, 5000) ; return 1 ; }
  else
      printf("[Input] Unknown command in input file |%s|\n", line) ;
return 0 ;
}
