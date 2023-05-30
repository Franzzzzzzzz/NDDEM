/** \addtogroup DEM Discrete Element Simulations
 * This module handles the Discrete Element Simulations.
 *  @{ */

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
/** \brief Manual multiprocessor handling for OpenMP, for max efficiency & avoiding bugs & race conditions, hopefully. 
 */
template <int d>
class Multiproc
{
public:
  
  void initialise (int NN, int PP, Parameters<d> & Param) {
    share.resize(PP+1,0) ; 
    num_time=0 ; 
    P=PP ; 
    N=NN ; 
    share[share.size()-1]=NN ;
    split(N,P) ;
    disp_share() ;
    CLp.resize(P,ContactList<d>()) ;
    CLw.resize(P,ContactList<d>()) ;
    CLm.resize(P,ContactListMesh<d>()) ; 
    C.resize(P, Contacts<d>(Param)) ;
    delayed.resize(P) ;
    delayedj.resize(P) ;
    delayed_size.resize(P,0) ;
    delayedwall.resize(P) ;
    delayedwallj.resize(P) ;
    delayedwall_size.resize(P,0) ;
    timing.resize(P,0) ;
    }
  void disp_share(); ///< Display the # of particle on each thread. 
  bool ismine (int ID, int j) {if (j>=share[ID] && j<share[ID+1]) return true ; return false ; } ///< Check if a particle j belongs to the thread ID. 
  void delaying (int ID, int j, Action<d> & act) ; ///< Record the action to be added later to the relevent atom in sequencial settings, avoid potential race condition if an action was added to an atom that is not owned by the thread. This is for a particle-particle contact
  void delayed_clean() ; ///< Clean the record list. 
  void delayingwall (int ID, int j, Action<d> & act) ; ///< Record the action on the wall. Only usefull if the force on the wall needs to be calculated
  void delayedwall_clean() ; ///< Clean the record of the force on the wal. 
  void load_balance() ; ///< Modify the atom share between threads to achieve better load balance between the threads based on the current speed of each one during the previous iterations. 

  vector <ContactList<d>> CLp ; ///< ContactList particle-particle for each processor
  vector <ContactList<d>> CLw ; ///< ContactList particle-wall for each processor
  vector <ContactListMesh<d>> CLm ; ///< ContactList particle-mesh for each processor
  vector <Contacts<d>> C ; ///< Dummy Contacts for independent calculation per processor
  vector <int> share ; ///< Particle share between threads. A thread ID own particles with index between share[ID] and share[ID+1]. size(share)=d+1. 
  vector <double> timing ; ///< Used to record the time spent by each thread. 
  int num_time ; ///< Number of sample of time spent. Resets when load_balance() is called. 

  // Array for temporary storing the reaction forces in the parallel part, to run sequencially after, to avoid data race
  vector <vector <Action<d>> > delayed ; ///< Records the delayed Action
  vector <vector <int> > delayedj ; ///< Records the j id of the particle in the associated delayed action
  vector <uint> delayed_size ; ///< Max length of the delayed vector for each thread. Can grow as needed on call to delaying()

  vector <vector <Action<d>> > delayedwall ; ///< Records the delayed Action
  vector <vector <int> > delayedwallj ; ///< Records the j id of the wall in the associated delayed action
  vector <uint> delayedwall_size ; ///< Max length of the delayed wall vector for each thread. Can grow as needed on call to delaying()

  auto contacts2array (ExportData exp, cv2d &X, cv2d &Boundaries) ; ///< pack the contact data in a 2d array
  
  int P ; ///< Number of threads

private:
  v2d fullcontactinfo; 
  int N ; ///< Number of grains
  void split (int N, int P) ; ///< Function to allocate the grains to threads taking into account the load balance in the contact detection. load_balance() takes over after a few iteration have run, and is usually more efficient.
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
void Multiproc<d>::delaying (int ID, int j, Action<d> & act)
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
void Multiproc<d>::delayingwall (int ID, int j, Action<d> & act)
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
//--------------------------------------------
template <int d>
void Multiproc<d>::load_balance()
{
  if (P==1) return ;
  double target = std::accumulate(timing.begin(), timing.end(), 0.) / P / num_time ;
  bool doloadbalance = false ;
  for (int i=0 ; i<P ; i++) // Checking that it's worth balancing the load
  {
    if (abs(1-timing[i]/num_time/target)>0.1)
      doloadbalance = true ;
  }
  if (!doloadbalance)
    return ;



  vector <double> timeperatom ;
  for (int i=0 ; i<P ; i++)
    timeperatom.push_back( (timing[i] / (share[i+1]-share[i])) / num_time) ;

  double time ;
  //for (auto v: timing) printf("%g ", v) ;
  vector <int> newshare(P+1,0) ;

  int curp=1 ;
  for (int p=1 ; p<P ; p++)
  {
    //printf("%d\n", newshare.size()) ; fflush(stdout) ;
    time=0 ; newshare[p] = newshare[p-1] ;
    //printf("%g\n", target) ; fflush(stdout) ;
    while (time<target)
    {
      if (newshare[p] >= share[curp])
        curp++ ;
      newshare[p]++ ;
      if (newshare[p]==N-(P-p)) break ; // To ensure at least 1 particle per core, not taking any risk
      time += timeperatom[curp-1] ;
    }
  }
  newshare[P] = N;

  /* For testing purposes (random allocation to proc)
  newshare[0] = 0 ; newshare[P]=N ;
  for (int i=1; i<P ; i++)
  {
    do {
    newshare[i] = newshare[i-1]+rand()%(N-newshare[i-1]-1)+1 ;
  } while (newshare[i]>=N-(P-i)) ;
  }*/

  printf("\nBalancing the load: processor migration\n") ;
  for (int i=0 ; i<P ; i++) printf("%d %ld |", share[i], CLp[i].v.size() ) ;
  printf("\n") ; fflush(stdout) ;

  share=newshare ;
  ContactList<d> temp_p, temp_w ;

  for (int i=0 ; i<P ; i++)
  {
    temp_p.v.splice(temp_p.v.end(), CLp[i].v) ;
    temp_w.v.splice(temp_w.v.end(), CLw[i].v) ;
  }

  //for (auto v: temp_p.v) printf("%d ", v.i) ;
  for (int p=0 ; p<P ; p++)
  {
    auto itp=temp_p.v.begin() ;
    while ( itp != temp_p.v.end() && itp->i < newshare[p+1]) itp++ ;
    CLp[p].v.splice(CLp[p].v.begin(), temp_p.v, temp_p.v.begin(), itp) ;

    auto itw=temp_w.v.begin() ;
    while ( itw != temp_w.v.end() && itw->i < newshare[p+1]) itw++ ;
    CLw[p].v.splice(CLw[p].v.begin(), temp_w.v, temp_w.v.begin(), itw) ;

  }

  for (int i=0 ; i<P ; i++) printf("%d %ld |", share[i], CLp[i].v.size() ) ;
  printf("\n") ; fflush(stdout) ;
}
//--------------------------------------------------------------------------------
template <int d>
auto Multiproc<d>::contacts2array (ExportData exprt, cv2d &X, cv2d &Boundaries)
{
  vector<vector<double>> res ;
  vector<std::pair<ExportData, int>> contactmapping ; 
  
  int n=0 ; 
  for (auto & clp:CLp) n+= clp.v.size() ; 
  res.resize(n) ;
  ExportData expid= static_cast<ExportData>(1) ; 
  ExportData expall=exprt ;
  n=0 ; 
  while (static_cast<int>(expall)>0)
  {
    if (expall & static_cast<ExportData>(1))
    {
      if (expid & (ExportData::GHOSTMASK | ExportData::GHOSTDIR | ExportData::FT_FRICTYPE)) 
      { contactmapping.push_back({expid,n}) ; n+=1 ; }
      else if (expid & (ExportData::IDS)) 
      { contactmapping.push_back({expid,n}) ; n+=2 ; }
      else if (expid & (ExportData::CONTACTPOSITION | ExportData::FN | ExportData::FT | ExportData::BRANCHVECTOR |
                        ExportData::FN_EL | ExportData::FN_VISC | ExportData::FT_EL | ExportData::FT_VISC | ExportData::FT_FRIC)) 
      { contactmapping.push_back({expid,n}) ; n+=d ;} 
    }
    
    expall>>=1 ; expid<<=1 ;
  }
  for (auto & v: res) v.reserve(n) ; 
  
  n=0 ; 
  for (auto & clp : CLp)
  {
    for (auto & contact : clp.v)
    {  
       ExportData expid= static_cast<ExportData>(1) ; 
       ExportData expall=exprt ;
       auto [loc,branch] = contact.compute_branchvector(X,Boundaries) ; 
       while (static_cast<int>(expall)>0)
       {
         if (expall & static_cast<ExportData>(1))
           switch (expid)
           {
              case ExportData::IDS: res[n].push_back({static_cast<double>(contact.i)}) ; res[n].push_back({static_cast<double>(contact.j)}) ; break ; 
              case ExportData::CONTACTPOSITION: for (int dd=0 ; dd<d ; dd++) res[n].push_back(loc[dd]) ; break ; 
              case ExportData::FN: for (int dd=0 ; dd<d ; dd++) res[n].push_back(contact.infos->Fn[dd]) ; break ;
              case ExportData::FT: for (int dd=0 ; dd<d ; dd++) res[n].push_back(contact.infos->Ft[dd]) ; break ;
              case ExportData::GHOSTMASK: res[n].push_back(static_cast<double>(contact.ghost)) ; break ;
              case ExportData::GHOSTDIR : res[n].push_back(static_cast<double>(contact.ghostdir)) ; break ;
              case ExportData::BRANCHVECTOR: for (int dd=0 ; dd<d ; dd++) res[n].push_back(branch[dd]) ; break ;
              case ExportData::FN_EL: for (int dd=0 ; dd<d ; dd++) res[n].push_back(contact.infos->Fn_el[dd]); break ; 
              case ExportData::FN_VISC: for (int dd=0 ; dd<d ; dd++) res[n].push_back(contact.infos->Fn_visc[dd]); break ; 
              case ExportData::FT_EL: for (int dd=0 ; dd<d ; dd++) res[n].push_back(contact.infos->Ft_el[dd]); break ; 
              case ExportData::FT_VISC: for (int dd=0 ; dd<d ; dd++) res[n].push_back(contact.infos->Ft_visc[dd]); break ; 
              case ExportData::FT_FRIC: for (int dd=0 ; dd<d ; dd++) res[n].push_back(contact.infos->Ft_fric[dd]); break ; 
              case ExportData::FT_FRICTYPE: res[n].push_back(static_cast<double>(contact.infos->Ft_isfric)); break ;  
              default: break ;
           }
           
        expall>>=1 ; expid<<=1 ;
      }
      n++ ; 
    }
  }
   
  return std::make_pair(contactmapping, res) ; 
}
#endif

/** @} */
