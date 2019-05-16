#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>
#include <thread>

using namespace std ;

void foo (double t)
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
f=fopen("pipe", "r") ;
vector <double> test(6,0) ;
std::thread first ;


while(1)
{
  int n ; int i ;
  i=-1 ;
  do
  {
    i++ ;
    n=fscanf(f, "%c", line+i) ;
  } while (n>0) ;
  line[i]=0 ;
clearerr(f) ;

stringstream ss(line);
if (line[0]!=0)
{
  for (int i=0 ; i<6 ; i++)
    ss>>test[i] ;
  first = std::thread (foo, test[5]);
  auto id = first.native_handle() ;
  printf("%d|%g %g %g %g %g %g\n",n, test[0], test[1], test[2], test[3], test[4], test[5]) ; fflush(stdout) ;
  usleep(10000000) ;
  pthread_cancel(id);
  first.join() ;
  usleep(1000000) ;
}
usleep(1000000) ;
printf(".")  ;fflush(stdout) ; 
}

fclose(f);




}
