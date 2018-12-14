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
  Multiproc (int NN, int PP, Parameters Param) : share(PP+1,0),P(PP), N(NN) {
    share[share.size()-1]=NN ;
    split(N,P) ;
    disp_share() ;
    CLp.resize(P) ;
    CLw.resize(P) ;
    C.resize(P, Contacts(Param)) ;
    }
  void disp_share();

  vector <ContactList> CLp ;
  vector <ContactList> CLw ;
  vector <Contacts> C ;
  vector <int> share ;

private:
  int P, N ;
  void split (int N, int P) ;
} ;
