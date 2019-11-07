#ifndef MULTIPROC
#define MULTIPROC

#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <vector>
#include <ctime>

#include "Contacts.h"
#include "ContactList.h"
#include "Parameters.h"

using namespace std ;
template <int d>
class Multiproc
{
public:
  Multiproc (int NN, int PP, Parameters<d> & Param) : share(PP+1,0),P(PP), N(NN) {
    share[share.size()-1]=NN ;
    split(N,P) ;
    disp_share() ;
    CLp.resize(P,ContactList<d>()) ;
    CLw.resize(P,ContactList<d>()) ;
    C.resize(P, Contacts<d>(Param)) ;
    delayed.resize(P) ;
    delayedj.resize(P) ;
    delayed_size.resize(P,0) ;
    delayedwall.resize(P) ;
    delayedwallj.resize(P) ;
    delayedwall_size.resize(P,0) ;
    }
  void disp_share();
  bool ismine (int ID, int j) {if (j>=share[ID] && j<share[ID+1]) return true ; return false ; }
  void delaying (int ID, int j, Action & act) ;
  void delayed_clean() ; 
  void delayingwall (int ID, int j, Action & act) ;
  void delayedwall_clean() ; 

  vector <ContactList<d>> CLp ;
  vector <ContactList<d>> CLw ;
  vector <Contacts<d>> C ;
  vector <int> share ;

  // Array for temporary storing the reaction forces in the parallel part, to run sequencially after, to avoid data race
  vector <vector <Action> > delayed ;
  vector <vector <int> > delayedj ;
  vector <uint> delayed_size ;
  
  vector <vector <Action> > delayedwall ; 
  vector <vector <int> > delayedwallj ; 
  vector <uint> delayedwall_size ; 

  int P ;

private:
  int N ;
  void split (int N, int P) ;
} ;

/*****************************************************************************************************
 *                                                                                                   *
 *                                                                                                   *
 *                                                                                                   *
 * IMPLEMENTATIONS                                                                                   *
 *                                                                                                   *
 *                                                                                                   *
 *                                                                                                   *
 * ***************************************************************************************************/

template <int d>
void Multiproc<d>::disp_share()
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
template <int d>
void Multiproc<d>::split(int N, int P)
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
template <int d>
void Multiproc<d>::delaying (int ID, int j, Action & act)
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
template <int d>
void Multiproc<d>::delayingwall (int ID, int j, Action & act)
{
  delayedwall_size[ID]++ ;
  if (delayedwall_size[ID] < delayedwallj[ID].size())
  {
    delayedwall[ID][delayedwall_size[ID]-1]=act ;
    delayedwallj[ID][delayedwall_size[ID]-1]=j ;
  }
  else
  {
    delayedwall[ID].resize(delayedwall_size[ID]+100, act) ;
    delayedwallj[ID].resize(delayedwall_size[ID]+100, j) ;
    //delayed[ID].push_back(act) ;
    //delayedj[ID].push_back(j) ;
  }
}
//---------------------------------------------
template <int d>
void Multiproc<d>::delayed_clean()
{
  for (auto & val : delayed_size)
  {
    val=0 ;
  }
}
template <int d>
void Multiproc<d>::delayedwall_clean()
{
  for (auto & val : delayedwall_size)
  {
    val=0 ;
  }
}


#endif
