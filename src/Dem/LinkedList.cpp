#include <cstdlib>
#include <cstdio>
#include "LinkedList.h"
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

boost::random::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));
boost::random::uniform_int_distribution<> action(0, 100);

template<typename T>
T random()
{
  
}


template <typename T>
randomize(std::list<T> &A, fg::list<T> &B, int target_size)
{
  assert((A.size() == B.size())) ; 
  
  boost::random::uniform_int_distribution<> where(0, A.size());
  
  size_t sz = target_size-A.size() ;
  if (sz>=target_size) p=0.5 ; 
  else if (p<=target_size/2) p=1 ; 
  else p=0.5+sz/(double)target_size/4 ; 
  
  int act = action(rng) ; 
  if (act>p*100)
  { // erase
    int id=where(rng) ; 
    auto i = A.begin() ; 
    for (int idd=id ; idd; i++,idd--) ; 
    A.erase(i) ; 
    
    auto i = B.begin() ; 
    for (int idd=id ; idd; i++,idd--) ; 
    B.erase(i) ; 
    
    delete ; 
  }
  else
  { //insert
    int id=where(rng) ; 
    T value = random<T>() ; 
    
    auto i = A.begin() ; 
    for (int idd=id ; idd; i++,idd--) ; 
    A.insert(value) ; 
    
    auto i = B.begin() ; 
    for (int idd=id ; idd; i++,idd--) ; 
    B.insert(value) ; 
    
    delete ; 
  }
  
  
}













int main(int argc, char * argv[]) 
{
  fg::list<int> test(100) ;
    
  test.insert(test.begin(), 1) ; 
  test.insert(test.begin(), 2) ; 
  test.insert(test.begin(), 3) ; 
  test.insert(test.begin(), 4) ; 
  test.insert(test.begin(), 5) ; 
  test.insert(test.end(), 6) ; 
  test.insert(test.end(), 7) ; 
  test.insert(test.end(), 8) ; 
  test.insert(test.end(), 9) ; 
  test.insert(test.end(), 10) ; 
  
  auto it=test.begin() ; 
  it++ ; it++ ; it++ ; it++;
  test.insert(it, 42) ; 

  std::vector<int> res ; 
  for (auto it = test.begin() ; it!=test.end() ; it++)
    res.push_back(*it) ; 
  if (res!=std::vector<int>({5,4,3,2,42,1,6,7,8,9,10})) {printf("TEST FAILED!\n") ; }
  else printf("TEST PASSED!\n") ; 
  
  if (res.size()!=11) {printf("TEST FAILED!\n") ; }
  else printf("TEST PASSED!\n") ; 
 
  test.erase(test.begin()) ; 
  it=test.begin() ; 
  it++ ; it++ ; it++ ; it++;
  test.erase(it) ; 
  
  res.clear() ; 
  for (auto it = test.begin() ; it!=test.end() ; it++)
    res.push_back(*it) ; 
  if (res!=std::vector<int>({4,3,2,42,6,7,8,9,10})) {printf("TEST FAILED!\n") ; }
  else printf("TEST PASSED!\n") ; 
  
  if (res.size()!=9) {printf("TEST FAILED!\n") ; }
  else printf("TEST PASSED!\n") ; 
  
  for (int i=0 ; i<res.size()+1 ; i++) test.erase(test.begin()) ;

  for (auto & v: test.freed) printf("%lX ", v) ; 
  
  
  return 0 ;  
}
