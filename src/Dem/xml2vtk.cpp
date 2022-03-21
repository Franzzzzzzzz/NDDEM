#include <cmath>
#include <cstdio>
#include <experimental/filesystem>
#include "Xml.h"


int main (int argc, char *argv[])
{
  if (argc!=2) {printf("Not the right number of argument, expecting a single filename (xml)\n") ; std::exit(1) ; }

  string path = argv[1] ;
  XMLReader input(path) ;

  char basicpath[5000] ;
  argv[1][strlen(argv[1])-4]=0 ;
  if (! experimental::filesystem::exists(argv[1])) experimental::filesystem::create_directory(argv[1]);

  int d = stoi(input.tags.second["dimensions"],0) ;
  int res ;

  string name ; map <string, string> attributes ;
  auto boundaries = input.gettagdata() ;
  auto radius = input.gettagdata() ;

  vector<vector<vector<double>>> result ;
  vector<string> names ;

  int n=0 ;
  while (!input.fic.eof() && res==0)
  {

    result.clear() ;
    res=input.read_nextts (names, result) ;
    if (res==1) continue ;

    if (names[0] != "Position") printf("Error: expecting position as 1st set of data") ;


    FILE *out ; static bool warn = false ;
    vector <float> projectioncenter  ; for (uint i=3 ; i<d ; i++) projectioncenter.push_back(((get<2>(boundaries))[2*i+1]+(get<2>(boundaries))[2*i])/2) ;

    char pathing[5000] ;
    sprintf(pathing, "%s/dump-%04d.vtk", argv[1], n) ; n++ ;
    out=fopen(pathing, "w") ; if (out==NULL) {printf("Cannot open out file\n") ; return 2 ;} //TODO

    /*if (d>3 && warn==false) {
        printf("WARN: writevtk might misbehave with dimension higher than 3. The 3d projection is always centered in all other dimensions\n") ;
        warn=true ;
    }*/
    fprintf(out, "# vtk DataFile Version 2.0\nMost Useless DEM (tm) output file\nASCII\nDATASET POLYDATA\n") ;

    vector<vector<double>> &X = result[0] ;
    fprintf(out, "POINTS %ld float\n", X.size()) ;
    for (uint i=0 ; i<X.size() ; i++) fprintf(out, "%g %g %g\n", X[i][0], X[i][1], d<3?0:X[i][2]) ;
    fprintf(out, "VERTICES %ld %ld\n", X.size(), 2*X.size()) ;
    for (uint i=0 ; i<X.size() ; i++) fprintf(out, "1 %d\n", i) ;

    fprintf(out, "\nPOINT_DATA %ld", get<2>(radius).size()) ;

    for (uint j=3 ; j<X[0].size() ; j++)
    {
      fprintf(out, "\nSCALARS Dimension%d float 1 \nLOOKUP_TABLE default \n", j) ;
      for (uint i=0 ; i<X.size() ; i++)
        fprintf(out, "%g ", X[i][j]) ;
    }

    fprintf(out, "\n\nSCALARS RadiusProjected float 1 \nLOOKUP_TABLE default\n");
    for (uint i=0 ; i<get<2>(radius).size() ; i++)
    {
      float value = get<2>(radius)[i]*get<2>(radius)[i] ;
      for (uint j=3 ; j<d ; j++) value-=(X[i][j]-projectioncenter[j-3])*(X[i][j]-projectioncenter[j-3]) ;
      if (value<0) fprintf(out, "%g ", 0.0) ;
      else fprintf(out, "%g ", sqrt(value)) ;
    }

    fclose(out) ;
  }
}
