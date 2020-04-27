#include <cstdlib>
#include <cstdio>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <sstream>
#include <functional>

using namespace std ;

double dst (vector <double> & a, int d ) {double v=0 ; for (int dd=0 ; dd<d ; dd++) v+=a[dd]*a[dd] ; return (sqrt(v)) ;  }

double numerical_integrate (int d, int nx)
{
  printf("Starting integration d=%d ... ", d) ;
  int count =0 ;
  vector <double> location (d) ;

  double dvol = pow(2./nx,d) ;
  double initloc=-1+1./nx ;
  for (auto & v:location) v=initloc ;

  std::function<void (int)> lbd ;
  double sum=0 ;

  lbd = [&](int dim)
  {
    for (int i=0 ; i<nx ; i++)
    {
      if (dim<d-1)
      {
        location[dim+1]=initloc ;
        lbd(dim+1) ;
      }
      else
      {
        //printf("%g %g %g\n", location[0], location[1], location[2]) ;
        double x=dst(location, d) ;
        if (x>=1) x=0 ;
        else {x= -3*x*x*x*x + 8*x*x*x -6*x*x +1 ; count ++ ; }
        sum += x*dvol ;
      }
      location[dim] += 2./nx ;
    }

  } ;
  lbd(0) ;

  printf("%10g Finished integration.\n", sum) ;
  return sum ;
}



int main(int argc, char *argv[])
{
int N=100 ;
  for (int i=1 ; i<32 ; i++)
  {
    double res = numerical_integrate(i, N) ;
    FILE * out = fopen("LucyCoeff.txt", "a") ;
    fprintf(out, "%d [%d]: %.17f\n", i, N, res) ;
    fclose(out) ;
  }

}
