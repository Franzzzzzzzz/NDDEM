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
class Multiproc
{
public:
  Multiproc (int NN, int PP, Parameters & Param) : share(PP+1,0),P(PP), N(NN) {
    share[share.size()-1]=NN ;
    split(N,P) ;
    disp_share() ;
    CLp.resize(P,ContactList(Param.d)) ;
    CLw.resize(P,ContactList(Param.d)) ;
    C.resize(P, Contacts(Param)) ;
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

  vector <ContactList> CLp ;
  vector <ContactList> CLw ;
  vector <Contacts> C ;
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

#endif
