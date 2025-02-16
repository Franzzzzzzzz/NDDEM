#include <cstdlib>
#include <cstdio>
#include "LinkedList.h"

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
