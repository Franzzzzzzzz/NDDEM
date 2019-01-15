#include "Multiproc.h"


void Multiproc::disp_share()
{
  printf("Processor share [%d threads]:\n", P) ;
  for (auto i : share)
    printf("%d ", i) ;
  printf("\n") ;
  for (int i=0 ; i<P ; i++)
    printf("%.1f ", ((N-share[i]- (share[i+1]-share[i])/2)*(share[i+1]-share[i]))/double((N*N-1)/2)*100) ;
  printf("\n--------\n") ;
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

share[P]=N ;
}

//-------------------------------------------------
void Multiproc::delaying (int ID, int j, Action & act)
{
  delayed_size[ID]++ ;
  if (delayed_size[ID] < delayedj[ID].size())
  {
    delayed[ID][delayed_size[ID]-1]=act ;
    delayedj[ID][delayed_size[ID]-1]=j ;
  }
  else
  {
    delayed[ID].resize(delayed_size[ID]+100, act) ;
    delayedj[ID].resize(delayed_size[ID]+100, j) ;
    //delayed[ID].push_back(act) ;
    //delayedj[ID].push_back(j) ;
  }
}
//---------------------------------------------
void Multiproc::delayed_clean()
{
  for (auto & val : delayed_size)
  {
    val=0 ;
  }
}
