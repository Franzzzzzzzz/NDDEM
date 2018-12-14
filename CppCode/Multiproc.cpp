#include "Multiproc.h"


void Multiproc::disp_share()
{
  printf("Processor share [%d threads]:\n", P) ;
  for (auto i : share)
    printf("%d ", i) ;
  for (int i=0 ; i<P ; i++)
    printf("%.2g", ((N-share[i]- (share[i+1]-share[i])/2)*(share[i+1]-share[i]))/double((N*N-1)/2*P)*100) ;
}
//=====================================================
void Multiproc::split(int N, int P)
{
double Delta,b1,b2 ;
int Nprime=N ;

double Area=N*(N-1)/(2*P) ;

for (int i=0 ; i<P ; i++)
{
  Delta=Nprime*Nprime-2*Area ;
  if (Delta<0) {printf("Error: Quadratic equation has no solution(multiproc)\n") ; fflush(stdout)  ;}
  b1=(Nprime-sqrt(Delta)) ;
  b2=(Nprime+sqrt(Delta)) ;
  if (b1>=0 && b1<N)
  {
    printf("%d Using b1 %d %d\n", i, int(floor(b1)), int(floor(b2))) ;
    share[i+1]=share[i]+int(floor(b1)) ;
    Nprime -= int(floor(b1)) ;
  }
  else if (b2>=0 && b2<N)
  {
    printf("%d Using b2 %d\n", i, int(floor(b2))) ;
    share[i+1]=share[i]+int(floor(b2)) ;
    Nprime -= int(floor(b2)) ;
  }
  else printf("No solution ...") ;
}

share[P]=N ; printf("\n\n") ;

for (int i=0 ; i<P ; i++) {
//printf("%d ", res[i]) ;
printf("%d ", (N-share[i]- (share[i+1]-share[i])/2)*(share[i+1]-share[i])) ;
}
}
