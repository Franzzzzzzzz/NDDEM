

#include <vector>
#include <cassert>
#include <cstdint>
#include <cmath>


namespace fg {

template <typename T>
class list {
public:  
  list(size_t nelements) {
    assert((sizeof(1ULL)*8 == 64)) ; 
    pool = (E*) malloc(sizeof(E)*nelements) ; 
    freed.resize(ceil(nelements/64.), ~0ULL) ; 
  }
  ~list() { free(pool) ; }
   
  
  struct E {
    T val ; 
    E* fwd ; 
    E* bwd ; 
  } ; 
  
  class iterator
  {
  public:
    E* el ;
    public: 
      iterator() {el=nullptr ; }
      iterator(E* element) {el=element;}
      bool operator==(iterator e) { return e.el==el ; }
      bool operator!=(iterator e) { return !(e.el==el) ; }
      iterator & operator++(int v) {el=el->fwd; return *this ; } // Not testing nullptr in increment and decrement for speed...
      iterator & operator--(int v) {el=el->bwd; return *this ; }
      T& operator*() {return el->val ; }
      E* operator->() {return el ; }
      E* raw() { return el ; }
    
  } ;
  
  E * pool ; 
  E* beg = nullptr ; 
  E* lastelement = nullptr ; 
  E* ending = nullptr ;
  size_t n = 0 ; 
    
  size_t size() { return n ; }
  iterator begin() {return {beg} ; }
  iterator end() {return {ending} ; }
  iterator insert(iterator pos, T && value) 
  { // Insert BEFORE the element
    auto new_element = allocate() ; 
    n ++ ;
    new_element->val = value ; 
    
    if (pos.raw() == ending)
    { 
      new_element->fwd = nullptr ;
      new_element->bwd = lastelement ;
      if (n==1) 
      {
        beg=new_element ;
        lastelement = new_element ; 
      }
      else
      {
        lastelement->fwd = new_element ;
        lastelement = new_element ; 
      }
    }
    else if (pos.raw() == beg)
    {
      new_element->fwd = beg ;
      new_element->bwd = nullptr ; 
      pos->bwd = new_element ; 
      beg = new_element ; 
    }
    else
    {
      new_element->fwd = pos.raw() ;
      new_element->bwd = pos->bwd ; 
      pos->bwd = new_element ; 
      (new_element->bwd)->fwd = new_element ; 
    }
    
    return {new_element} ;
  }
  E* erase(iterator el)
  {
    E* ret = nullptr ; 
    if (el==nullptr) return nullptr ; 
    
    if (n==0 && el != nullptr)
    {
      printf("ERR: cannot remove from empty list\n") ; 
      return nullptr ; 
    }
    else if (n==1) 
    {
      beg=lastelement=ending ; 
    }
    else
    {    
      if (el->fwd == ending)
      {
        (el->bwd)->fwd = ending ;
        lastelement = el->bwd ; 
      }
      else if (el->bwd == nullptr)
      {
        (el->fwd)->bwd = nullptr ; 
        beg = el->fwd ; 
      }
      else
      {
        (el->fwd)->bwd = el->bwd ; 
        (el->bwd)->fwd = el->fwd ; 
      }
      ret = el->fwd ; 
    }
      
    freemem(el) ; 
    n-- ;
    
    return ret ; 
  }
  

//private:
  std::vector<uint64_t> freed ;  
  
  E * allocate() {
    size_t i,j ; 
    // TODO use a mutex. This need to be atomic from here ...
    for (i = 0 ; i<freed.size() && freed[i]==0 ; ++i) ; 
    if (i==freed.size()) {printf("ERROR: no more space to allocate element in the list\n") ; return nullptr ; }
    uint64_t v = freed[i] ; 
    for (j = 0 ; (v & 1ULL) == 0 ; v>>=1, j++) ;
    freed[i] &= ~(1ULL << j) ;     
    // ... to there
    return pool + (i*64+j) ; 
  }
  
  void freemem (iterator el)
  {
    size_t delta = el.raw()-pool ; 
    int i = delta/64 ; 
    int j = delta%64 ; 
    freed[i] |= 1ULL<<j ; 
  }
  
  
} ; 
  
  
  
  
  
  
  
  
  
  
  
  
} ; 







