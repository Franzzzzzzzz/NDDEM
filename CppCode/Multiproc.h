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
    CLp.resize(P) ;
    CLw.resize(P) ;
    C.resize(P, Contacts(Param)) ;
    tmp_j.resize(P) ; 
    tmp_forcen.resize(P) ; 
    tmp_forcet.resize(P) ; 
    tmp_torque.resize(P) ; 
    }
  void disp_share();
  
  vector <ContactList> CLp ;
  vector <ContactList> CLw ;
  vector <Contacts> C ;
  vector <int> share ;

  // Array for temporary storing the reaction forces in the parallel part, to run sequencially after, to avoid data race
  vector<vector<int>> tmp_j ; 
  vector<vector<vector<double>>> tmp_forcen ;
  vector<vector<vector<double>>> tmp_forcet ;
  vector<vector<vector<double>>> tmp_torque ; 
  
  int P ;
  
private:
  int N ;
  void split (int N, int P) ;
} ;
