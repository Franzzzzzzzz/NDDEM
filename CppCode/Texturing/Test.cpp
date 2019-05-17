#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>
#include <thread>

using namespace std ;

void foo (double t, double a)
{
  for (int i=0 ; i<t ; i++)
  {
    printf("A") ; fflush(stdout) ;
    usleep(1000000) ;
  }
}






int main (int argc, char * argv[])
{
FILE * f ;char letter ;
char line[5000] ;
//f=fopen("pipe", "r") ;
vector <double> test(12,0) ;
std::thread first ;

printf("Hello") ; fflush(stdout) ;
if (first.joinable())
  first.join() ;
printf("Here") ; fflush(stdout) ;
test[5]=0 ;
first = std::thread (foo, test[5], test[2]);
usleep(1000000) ;
if (first.joinable())
{
printf("BOO") ; fflush(stdout) ;
first.join() ;
}




}
